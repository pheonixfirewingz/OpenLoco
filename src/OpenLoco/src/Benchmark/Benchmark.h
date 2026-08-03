#pragma once
#include <array>
#include <chrono>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace OpenLoco::Benchmark
{
    inline constexpr std::string_view kResultSchema = "openloco-benchmark-result/v1";
    struct Sample
    {
        std::string phase;
        std::string component;
        std::string camera;
        uint32_t firstIteration{};
        uint32_t iterationCount{};
        uint64_t nanoseconds{};
    };
    class Collector
    {
    public:
        void beginWarmup(uint32_t iterations);
        void beginMeasurement();
        void add(Sample sample);
        const std::vector<Sample>& samples() const;
        uint32_t warmupIterations() const;

    private:
        uint32_t _warmupIterations{};
        bool _measuring{};
        std::vector<Sample> _samples;
    };

    enum class LogicComponent : uint8_t
    {
        tiles,
        waves,
        towns,
        industries,
        vehicles,
        stations,
        effects,
        companies,
        animations,
        audioState,
        titleLogic,
        count
    };
    void setLogicBreakdownEnabled(bool enabled);
    bool isLogicBreakdownEnabled();
    void resetLogicBreakdown();
    void recordLogicDuration(LogicComponent component, uint64_t nanoseconds);
    const std::array<uint64_t, static_cast<size_t>(LogicComponent::count)>& logicBreakdown();
    std::string_view logicComponentName(LogicComponent component);
    enum class RenderComponent : uint8_t
    {
        dirtyRegions,
        viewportUpdates,
        viewportWorld,
        uiWindow,
        overlay,
        count
    };
    void setRenderBreakdownEnabled(bool enabled);
    bool isRenderBreakdownEnabled();
    void resetRenderBreakdown();
    void recordRenderDuration(RenderComponent component, uint64_t nanoseconds);
    const std::array<uint64_t, static_cast<size_t>(RenderComponent::count)>& renderBreakdown();
    std::string_view renderComponentName(RenderComponent component);
    template<typename F>
    void measureRender(const RenderComponent component, F&& fn)
    {
        if (!isRenderBreakdownEnabled())
        {
            fn();
            return;
        }
        const auto start = std::chrono::steady_clock::now();
        fn();
        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
        recordRenderDuration(component, static_cast<uint64_t>(elapsed));
    }
    int runCaseFile(const std::string& casePath, const std::string& outputPath);
    int runSelfTests();
    bool isActive();
    uint64_t fnv1a64(std::span<const uint8_t> bytes, uint64_t seed = 14695981039346656037ULL);

    template<typename F>
    void measureLogic(const LogicComponent component, F&& fn)
    {
        if (!isLogicBreakdownEnabled())
        {
            fn();
            return;
        }
        const auto start = std::chrono::steady_clock::now();
        fn();
        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
        recordLogicDuration(component, static_cast<uint64_t>(elapsed));
    }
}
