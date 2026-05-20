#include "PngImage.h"

#include <OpenLoco/Diagnostics/Logging.h>
#include <cassert>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace OpenLoco::Diagnostics;

namespace OpenLoco::Gfx
{
    PngImage::PngImage(int w, int h, int c)
        : width(w)
        , height(h)
        , channels(c)
    {
        assert(w > 0);
        assert(h > 0);
        assert(c > 0);

        imageData = std::vector<unsigned char>(w * h * c);
    }

    Colour32 PngImage::getPixel(int x, int y)
    {
        const size_t index = (y * width + x) * channels;
        assert(index + channels <= imageData.size());

        return {
            imageData[index + 0],
            imageData[index + 1],
            imageData[index + 2],
            imageData[index + 3]
        };
    }

    std::unique_ptr<PngImage> PngImage::loadFromFile(const std::filesystem::path& filePath)
    {
        int w, h, channels;

        // Load image using stb_image (always request 4 channels for RGBA)
        unsigned char* data = stbi_load(filePath.string().c_str(), &w, &h, &channels, 4);

        if (!data)
        {
            Logging::error("Failed to load PNG file: {}", filePath.string());
            return nullptr;
        }

        // Ensure we have 4 channels (RGBA)
        channels = 4;

        auto pngImage = std::make_unique<PngImage>(w, h, channels);

        // Copy data to the image
        std::memcpy(pngImage->imageData.data(), data, w * h * channels);

        // Free stb_image data
        stbi_image_free(data);

        return pngImage;
    }
}
