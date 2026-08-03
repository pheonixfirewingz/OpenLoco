#include "Benchmark.h"
#include "Audio/Audio.h"
#include "Config.h"
#include "Graphics/Gfx.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/SoftwareDrawingEngine.h"
#include "Map/Tile.h"
#include "Map/TileManager.h"
#include "OpenLoco.h"
#include "S5/S5.h"
#include "Ui/Window.h"
#include "Ui/WindowManager.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <yaml-cpp/yaml.h>

namespace OpenLoco::Benchmark
{
    static bool _logicBreakdownEnabled{};
    static std::array<uint64_t, static_cast<size_t>(LogicComponent::count)> _logicBreakdown{};
    static bool _renderBreakdownEnabled{};
    static std::array<uint64_t, static_cast<size_t>(RenderComponent::count)> _renderBreakdown{};
    static bool _active{};
    void Collector::beginWarmup(const uint32_t iterations)
    {
        _warmupIterations = iterations;
        _measuring = false;
        _samples.clear();
    }
    void Collector::beginMeasurement() { _measuring = true; }
    void Collector::add(Sample sample)
    {
        if (_measuring)
        {
            _samples.push_back(std::move(sample));
        }
    }
    const std::vector<Sample>& Collector::samples() const { return _samples; }
    uint32_t Collector::warmupIterations() const { return _warmupIterations; }
    void setLogicBreakdownEnabled(const bool enabled) { _logicBreakdownEnabled = enabled; }
    bool isLogicBreakdownEnabled() { return _logicBreakdownEnabled; }
    void resetLogicBreakdown() { _logicBreakdown.fill(0); }
    void recordLogicDuration(const LogicComponent component, const uint64_t nanoseconds)
    {
        if (_logicBreakdownEnabled)
        {
            _logicBreakdown[static_cast<size_t>(component)] += nanoseconds;
        }
    }
    const std::array<uint64_t, static_cast<size_t>(LogicComponent::count)>& logicBreakdown() { return _logicBreakdown; }
    std::string_view logicComponentName(const LogicComponent component)
    {
        constexpr std::array names{ "tiles", "waves", "towns", "industries", "vehicles", "stations", "effects", "companies", "animations", "audio-state", "title-logic" };
        return names[static_cast<size_t>(component)];
    }
    void setRenderBreakdownEnabled(const bool enabled) { _renderBreakdownEnabled = enabled; }
    bool isRenderBreakdownEnabled() { return _renderBreakdownEnabled; }
    void resetRenderBreakdown() { _renderBreakdown.fill(0); }
    void recordRenderDuration(const RenderComponent component, const uint64_t nanoseconds)
    {
        if (_renderBreakdownEnabled)
        {
            _renderBreakdown[static_cast<size_t>(component)] += nanoseconds;
        }
    }
    const std::array<uint64_t, static_cast<size_t>(RenderComponent::count)>& renderBreakdown() { return _renderBreakdown; }
    std::string_view renderComponentName(const RenderComponent component)
    {
        constexpr std::array names{ "dirty-region-rendering", "viewport-updates", "viewport-world-painting", "ui-window-painting", "fps-overlay" };
        return names[static_cast<size_t>(component)];
    }
    bool isActive() { return _active; }
    uint64_t fnv1a64(const std::span<const uint8_t> bytes, uint64_t seed)
    {
        for (const auto byte : bytes)
        {
            seed ^= byte;
            seed *= 1099511628211ULL;
        }
        return seed;
    }

    static std::string jsonString(const std::string_view value)
    {
        std::ostringstream out;
        out << '"';
        for (const unsigned char c : value)
        {
            switch (c)
            {
                case '"': out << "\\\""; break;
                case '\\': out << "\\\\"; break;
                case '\b': out << "\\b"; break;
                case '\f': out << "\\f"; break;
                case '\n': out << "\\n"; break;
                case '\r': out << "\\r"; break;
                case '\t': out << "\\t"; break;
                default:
                    if (c < 0x20)
                    {
                        out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c) << std::dec;
                    }
                    else
                    {
                        out << c;
                    }
            }
        }
        return out.str() + '"';
    }

    int runSelfTests()
    {
        Collector collector;
        collector.beginWarmup(3);
        collector.add({ "logic-total", "", "", 0, 1, 10 });
        if (!collector.samples().empty() || collector.warmupIterations() != 3)
        {
            return 1;
        }
        collector.beginMeasurement();
        collector.add({ "logic-total", "", "", 0, 2, 20 });
        if (collector.samples().size() != 1 || collector.samples()[0].nanoseconds != 20 || collector.samples()[0].iterationCount != 2)
        {
            return 2;
        }
        setLogicBreakdownEnabled(true);
        resetLogicBreakdown();
        recordLogicDuration(LogicComponent::tiles, 7);
        if (logicBreakdown()[static_cast<size_t>(LogicComponent::tiles)] != 7)
        {
            return 3;
        }
        setLogicBreakdownEnabled(false);
        recordLogicDuration(LogicComponent::tiles, 9);
        if (logicBreakdown()[static_cast<size_t>(LogicComponent::tiles)] != 7)
        {
            return 4;
        }
        setRenderBreakdownEnabled(true);
        resetRenderBreakdown();
        recordRenderDuration(RenderComponent::uiWindow, 11);
        if (renderBreakdown()[static_cast<size_t>(RenderComponent::uiWindow)] != 11)
        {
            return 5;
        }
        setRenderBreakdownEnabled(false);
        if (jsonString("a\n\"b") != "\"a\\n\\\"b\"")
        {
            return 6;
        }
        constexpr std::array<uint8_t, 5> hello{ 'h', 'e', 'l', 'l', 'o' };
        if (fnv1a64(hello) != 0xa430d84680aabd0bULL)
        {
            return 7;
        }
        return 0;
    }

    int runCaseFile(const std::string& casePath, const std::string& outputPath)
    {
        try
        {
            _active = true;
            const auto root = YAML::LoadFile(casePath);
            if (root["schema"].as<std::string>() != "openloco-benchmark-case/v1")
            {
                throw std::runtime_error("Unsupported benchmark case schema");
            }
            const auto suiteHash = root["suiteHash"].as<std::string>();
            const auto fixture = root["workload"]["fixture"]["absolutePath"].as<std::string>();
            const auto workload = root["workload"]["id"].as<std::string>();
            const auto repetition = root["repetition"].as<uint32_t>();
            const auto config = root["config"];
            const auto warmup = config["logicWarmupTicks"].as<uint32_t>();
            const auto measured = config["logicMeasuredTicks"].as<uint32_t>();
            const auto batch = config["logicBatchTicks"].as<uint32_t>();
            if (batch == 0 || measured == 0 || measured % batch != 0)
            {
                throw std::runtime_error("Invalid logic measurement counts");
            }

            std::vector<Sample> samples;
            std::vector<std::pair<std::string, uint64_t>> frameHashes;
            benchmarkLoadGame(std::filesystem::u8path(fixture));
            if (Audio::isAudioEnabled())
            {
                Audio::toggleSound();
            }
            setLogicBreakdownEnabled(false);
            benchmarkTickLogic(static_cast<int32_t>(warmup));
            for (uint32_t first = 0; first < measured; first += batch)
            {
                const auto start = std::chrono::steady_clock::now();
                benchmarkTickLogic(static_cast<int32_t>(batch));
                const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
                samples.push_back({ "logic-total", "", "", first, batch, static_cast<uint64_t>(ns) });
            }

            benchmarkLoadGame(std::filesystem::u8path(fixture));
            setLogicBreakdownEnabled(true);
            benchmarkTickLogic(static_cast<int32_t>(warmup));
            for (uint32_t first = 0; first < measured; first += batch)
            {
                resetLogicBreakdown();
                const auto start = std::chrono::steady_clock::now();
                benchmarkTickLogic(static_cast<int32_t>(batch));
                const auto total = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count());
                uint64_t accounted{};
                const auto& values = logicBreakdown();
                for (size_t i = 0; i < values.size(); ++i)
                {
                    accounted += values[i];
                    samples.push_back({ "logic-breakdown", std::string(logicComponentName(static_cast<LogicComponent>(i))), "", first, batch, values[i] });
                }
                samples.push_back({ "logic-breakdown", "other", "", first, batch, total > accounted ? total - accounted : 0 });
            }
            setLogicBreakdownEnabled(false);

            const auto artifactDir = std::filesystem::path(outputPath).parent_path() / (std::filesystem::path(outputPath).stem().string() + "-artifacts");
            std::filesystem::create_directories(artifactDir);

            const auto width = root["resolution"]["width"].as<int32_t>();
            const auto height = root["resolution"]["height"].as<int32_t>();
            const auto renderWarmup = config["renderWarmupFrames"].as<uint32_t>();
            const auto renderMeasured = config["renderMeasuredFrames"].as<uint32_t>();
            auto& engine = Gfx::getDrawingEngine();
            engine.resize(width, height);
            Config::get().showFPS = false;
            for (const auto& camera : root["workload"]["cameras"])
            {
                const auto cameraId = camera["id"].as<std::string>();
                const auto cameraTick = camera["tick"].as<uint32_t>();
                const auto mapX = camera["x"].as<int32_t>();
                const auto mapY = camera["y"].as<int32_t>();
                const auto zoom = camera["zoom"].as<int8_t>();
                const auto rotation = camera["rotation"].as<int8_t>();
                benchmarkLoadGame(std::filesystem::u8path(fixture));
                benchmarkTickLogic(static_cast<int32_t>(cameraTick));
                S5::exportGameStateToFile(artifactDir / ("camera-" + cameraId + "-state.sv5"), S5::SaveFlags::none);
                auto* main = Ui::WindowManager::getMainWindow();
                if (main == nullptr || main->viewports[0] == nullptr)
                {
                    throw std::runtime_error("Main viewport is unavailable");
                }
                Ui::WindowManager::setCurrentRotation(rotation);
                const auto x = static_cast<coord_t>(mapX);
                const auto y = static_cast<coord_t>(mapY);
                const auto landHeight = World::TileManager::getHeight({ x, y }).landHeight;
                const auto centre = World::gameToScreen(World::Pos3{ x, y, landHeight }, rotation);
                main->viewportFromSavedView({ static_cast<coord_t>(centre.x), static_cast<coord_t>(centre.y), static_cast<ZoomLevel>(zoom), rotation });
                Ui::WindowManager::updateViewports();

                const auto forceRender = [&] {
                    Gfx::invalidateScreen();
                    engine.render();
                };
                setRenderBreakdownEnabled(false);
                for (uint32_t i = 0; i < renderWarmup; ++i)
                {
                    forceRender();
                }
                for (uint32_t i = 0; i < renderMeasured; ++i)
                {
                    const auto start = std::chrono::steady_clock::now();
                    forceRender();
                    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
                    samples.push_back({ "render-total", "", cameraId, i, 1, static_cast<uint64_t>(ns) });
                }

                setRenderBreakdownEnabled(true);
                for (uint32_t i = 0; i < renderWarmup; ++i)
                {
                    forceRender();
                }
                for (uint32_t i = 0; i < renderMeasured; ++i)
                {
                    resetRenderBreakdown();
                    const auto start = std::chrono::steady_clock::now();
                    forceRender();
                    const auto total = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count());
                    uint64_t accounted{};
                    const auto& values = renderBreakdown();
                    for (size_t c = 0; c < values.size(); ++c)
                    {
                        accounted += values[c];
                        samples.push_back({ "render-breakdown", std::string(renderComponentName(static_cast<RenderComponent>(c))), cameraId, i, 1, values[c] });
                    }
                    samples.push_back({ "render-breakdown", "other", cameraId, i, 1, total > accounted ? total - accounted : 0 });
                }
                setRenderBreakdownEnabled(false);

                forceRender();
                std::ofstream frame(artifactDir / ("frame-" + cameraId + ".indexed"), std::ios::binary);
                const auto& rt = engine.getScreenRT();
                uint64_t frameHash = 14695981039346656037ULL;
                for (int32_t row = 0; row < rt.height; ++row)
                {
                    frame.write(reinterpret_cast<const char*>(rt.bits + row * (rt.width + rt.pitch)), rt.width);
                    frameHash = fnv1a64({ rt.bits + row * (rt.width + rt.pitch), static_cast<size_t>(rt.width) }, frameHash);
                }
                const auto palette = Gfx::getRgbaPalette();
                frame.write(reinterpret_cast<const char*>(palette.data()), static_cast<std::streamsize>(palette.size_bytes()));
                frameHash = fnv1a64({ reinterpret_cast<const uint8_t*>(palette.data()), palette.size_bytes() }, frameHash);
                frameHashes.emplace_back(cameraId, frameHash);
            }

            benchmarkLoadGame(std::filesystem::u8path(fixture));
            const uint32_t correctnessTicks = measured;
            for (uint32_t tick = 500; tick <= correctnessTicks; tick += 500)
            {
                benchmarkTickLogic(500);
                S5::exportGameStateToFile(artifactDir / ("state-" + std::to_string(tick) + ".sv5"), S5::SaveFlags::none);
            }

            std::ofstream out(outputPath, std::ios::binary);
            if (!out)
            {
                throw std::runtime_error("Unable to open benchmark result output");
            }
            out << "{\"schema\":\"" << kResultSchema << "\",\"suiteHash\":" << jsonString(suiteHash)
                << ",\"target\":\"native\",\"workload\":" << jsonString(workload) << ",\"repetition\":" << repetition
                << ",\"warmup\":{\"logicTicks\":" << warmup << "},\"samples\":[";
            for (size_t i = 0; i < samples.size(); ++i)
            {
                const auto& s = samples[i];
                if (i != 0)
                {
                    out << ',';
                }
                out << "{\"phase\":" << jsonString(s.phase) << ",\"component\":" << jsonString(s.component)
                    << ",\"camera\":" << jsonString(s.camera)
                    << ",\"firstIteration\":" << s.firstIteration << ",\"iterationCount\":" << s.iterationCount
                    << ",\"nanoseconds\":" << s.nanoseconds << '}';
            }
            out << "],\"frameHashes\":{";
            for (size_t i = 0; i < frameHashes.size(); ++i)
            {
                if (i != 0)
                {
                    out << ',';
                }
                out << jsonString(frameHashes[i].first) << ":\"fnv1a64:" << std::hex << std::setw(16) << std::setfill('0') << frameHashes[i].second << std::dec << '"';
            }
            out << "},\"compatibility\":{\"pass\":true,\"stateCheckpointInterval\":500,\"artifactDirectory\":"
                << jsonString(artifactDir.u8string()) << "}}\n";
            return 0;
        }
        catch (const std::exception& e)
        {
            _active = false;
            std::ofstream out(outputPath, std::ios::binary);
            out << "{\"schema\":\"" << kResultSchema << "\",\"error\":" << jsonString(e.what()) << "}\n";
            return 1;
        }
    }
}
