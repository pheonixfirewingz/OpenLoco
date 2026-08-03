#include "Benchmark.h"

#ifdef __EMSCRIPTEN__
#include <cstdlib>
#include <cstring>
#include <emscripten/emscripten.h>
#include <fstream>
#include <sstream>

extern "C" {
EMSCRIPTEN_KEEPALIVE int openloco_benchmark_write_fixture(const char* path, const uint8_t* bytes, const size_t length)
{
    std::ofstream out(path, std::ios::binary);
    if (!out)
    {
        return 1;
    }
    out.write(reinterpret_cast<const char*>(bytes), static_cast<std::streamsize>(length));
    return out ? 0 : 1;
}

EMSCRIPTEN_KEEPALIVE char* openloco_benchmark_run(const char* caseJson)
{
    constexpr auto casePath = "/tmp/openloco-benchmark-case.json";
    constexpr auto resultPath = "/tmp/openloco-benchmark-result.json";
    {
        std::ofstream out(casePath, std::ios::binary);
        out << caseJson;
    }
    OpenLoco::Benchmark::runCaseFile(casePath, resultPath);
    std::ifstream in(resultPath, std::ios::binary);
    std::ostringstream contents;
    contents << in.rdbuf();
    const auto value = contents.str();
    auto* result = static_cast<char*>(std::malloc(value.size() + 1));
    if (result == nullptr)
    {
        return nullptr;
    }
    std::memcpy(result, value.c_str(), value.size() + 1);
    return result;
}

EMSCRIPTEN_KEEPALIVE void openloco_benchmark_free(char* value)
{
    std::free(value);
}

EMSCRIPTEN_KEEPALIVE char* openloco_benchmark_read_base64(const char* path)
{
    static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        return nullptr;
    }
    const std::string bytes{ std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
    std::string encoded;
    encoded.reserve(((bytes.size() + 2) / 3) * 4);
    for (size_t i = 0; i < bytes.size(); i += 3)
    {
        const uint32_t a = static_cast<uint8_t>(bytes[i]);
        const uint32_t b = i + 1 < bytes.size() ? static_cast<uint8_t>(bytes[i + 1]) : 0;
        const uint32_t c = i + 2 < bytes.size() ? static_cast<uint8_t>(bytes[i + 2]) : 0;
        const uint32_t value = (a << 16) | (b << 8) | c;
        encoded += alphabet[(value >> 18) & 63];
        encoded += alphabet[(value >> 12) & 63];
        encoded += i + 1 < bytes.size() ? alphabet[(value >> 6) & 63] : '=';
        encoded += i + 2 < bytes.size() ? alphabet[value & 63] : '=';
    }
    auto* result = static_cast<char*>(std::malloc(encoded.size() + 1));
    if (result == nullptr)
    {
        return nullptr;
    }
    std::memcpy(result, encoded.c_str(), encoded.size() + 1);
    return result;
}
}
#endif
