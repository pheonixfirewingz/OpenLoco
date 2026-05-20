#include "OpenLoco/Audio/AudioEngine.h"
#include <OpenLoco/Diagnostics/Logging.h>

namespace OpenLoco::Audio
{
    using namespace Diagnostics;

    void initialize()
    {
        Logging::info("Audio not available in web build");
    }

    void shutdown()
    {
        // Nothing to do
    }

    bool openDevice([[maybe_unused]] const std::string& name)
    {
        return false;
    }

    void closeDevice()
    {
        // Nothing to do
    }

    std::vector<std::string> getAvailableDevices()
    {
        return {};
    }

    BufferId loadBuffer([[maybe_unused]] std::span<const uint8_t> pcmData, [[maybe_unused]] const AudioFormat& format)
    {
        return BufferId::null;
    }

    void unloadBuffer([[maybe_unused]] BufferId buffer)
    {
        // Nothing to do
    }

    AudioHandle create([[maybe_unused]] BufferId buffer, [[maybe_unused]] ChannelId channel, [[maybe_unused]] const AudioAttributes& attribs)
    {
        return AudioHandle::null;
    }

    void destroy([[maybe_unused]] AudioHandle handle)
    {
        // Nothing to do
    }

    void play([[maybe_unused]] AudioHandle handle)
    {
        // Nothing to do
    }

    void stop([[maybe_unused]] AudioHandle handle)
    {
        // Nothing to do
    }

    void pause([[maybe_unused]] AudioHandle handle)
    {
        // Nothing to do
    }

    void unpause([[maybe_unused]] AudioHandle handle)
    {
        // Nothing to do
    }

    bool isPlaying([[maybe_unused]] AudioHandle handle)
    {
        return false;
    }

    bool isPaused([[maybe_unused]] AudioHandle handle)
    {
        return false;
    }

    void setVolume([[maybe_unused]] AudioHandle handle, [[maybe_unused]] int32_t volume)
    {
        // Nothing to do
    }

    void setPan([[maybe_unused]] AudioHandle handle, [[maybe_unused]] int32_t pan)
    {
        // Nothing to do
    }

    void setPitch([[maybe_unused]] AudioHandle handle, [[maybe_unused]] int32_t frequency)
    {
        // Nothing to do
    }

    void setAttributes([[maybe_unused]] AudioHandle handle, [[maybe_unused]] const AudioAttributes& attribs)
    {
        // Nothing to do
    }

    void setChannelVolume([[maybe_unused]] ChannelId channel, [[maybe_unused]] int32_t volume)
    {
        // Nothing to do
    }

    int32_t getChannelVolume([[maybe_unused]] ChannelId channel)
    {
        return 0;
    }

    void setReverb([[maybe_unused]] AudioHandle handle, [[maybe_unused]] const ReverbParams& params)
    {
        // Nothing to do
    }

    void reclaimFinishedInstances()
    {
        // Nothing to do
    }

    void stopAll()
    {
        // Nothing to do
    }

    void pauseAll()
    {
        // Nothing to do
    }

    void unpauseAll()
    {
        // Nothing to do
    }

    bool isEnabled()
    {
        return false;
    }
}
