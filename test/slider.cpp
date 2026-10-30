#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <grvl/Painter.h>
#include <grvl/component/Slider.h>
#include <grvl/grvl.h>

#include <array>

namespace {
    struct SliderEnvironment {
        SliderEnvironment()
        {
            grvl::grvl::Init(nullptr);
        }

        ~SliderEnvironment()
        {
            grvl::grvl::Destroy();
        }
    };
}

TEST_CASE("Horizontal slider keeps its track width", "[component][slider]")
{
    SliderEnvironment environment;
    const int width = GENERATE(100, 101);
    const int value = GENERATE(0, 1, 25, 50, 75, 90, 99, 100);
    constexpr int displayWidth = 120;
    constexpr int displayHeight = 40;
    constexpr uint32_t background = 0xFF123456;
    constexpr uint32_t inactive = 0xFFFFFFFF;
    constexpr uint32_t active = 0xFFA903FC;
    constexpr uint32_t handle = 0xFFFCBA03;
    std::array<uint32_t, displayWidth * displayHeight> pixels;

    grvl::Painter painter;
    painter.SetDisplaySize(displayWidth, displayHeight);
    painter.CreateFramebuffersCollection(grvl::Format::ARGB8888, pixels.data(), pixels.data());
    painter.ResetDrawingBounds();

    grvl::Slider slider { };
    slider.SetPosition(3, 4);
    slider.SetSize(width, 20);
    slider.SetBackgroundColor(0);
    slider.SetFrameColor(0);
    slider.SetBarColor(inactive);
    slider.SetActiveBarColor(active);
    slider.SetScrollColor(handle);
    slider.SetValueRange(0, 100);
    slider.SetValue(value);

    // Include parent offsets and check both the initial and subsequent draws.
    constexpr int x = 3 + 2;
    constexpr int y = 4 + 3;
    for(int frame = 0; frame < 2; ++frame) {
        pixels.fill(background);
        slider.Draw(painter, 2, 3);
        for(int row = 0; row < displayHeight; ++row) {
            for(int column = 0; column < displayWidth; ++column) {
                INFO("width=" << width << ", value=" << value << ", frame=" << frame
                              << ", pixel=" << column << "," << row);
                const auto pixel = pixels[row * displayWidth + column];
                if(column >= x && column < x + width && row >= y && row < y + 20) {
                    REQUIRE((pixel == inactive || pixel == active || pixel == handle));
                } else {
                    REQUIRE(pixel == background);
                }
            }
        }
    }
}
