
#include <catch2/catch_test_macros.hpp>

#include <grvl/Font.h>
#include <grvl/Manager.h>
#include <grvl/component/TextInput.h>
#include <grvl/grvl.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <set>
#include <string>
#include <vector>

using namespace grvl;

namespace {

    constexpr uint32_t WIDTH { 120 };
    constexpr uint32_t HEIGHT { 40 };
    constexpr uint32_t CURSOR_RGB { 0x00ff00 };
    constexpr int32_t FONT_HEIGHT { 8 }; // mona16.gbf

    std::vector<uint8_t> framebuffer;

    // A clock the tests drive, so the blink phase is deterministic.
    uint64_t fakeClock { 0 };
    uint64_t FakeTimestamp()
    {
        return fakeClock;
    }

    // Builds a document holding one TextInput and renders one frame.
    TextInput* Render(const std::string& inputAttributes, bool focus)
    {
        framebuffer.assign(2 * WIDTH * HEIGHT * 4, 0);

        gui_callbacks_t callbacks { };
        callbacks.get_timestamp = FakeTimestamp;
        grvl::grvl::Init(&callbacks);

        Manager::Initialize(WIDTH, HEIGHT, 4, false, framebuffer.data());
        Manager& manager = Manager::GetInstance();
        manager.AddFontToFontContainer("normal", new GrvlBakedFont(GRVL_TEST_FONT));

        const std::string document = "<doc><stylesheet></stylesheet><customView id=\"home\" backgroundColor=\"#ff000000\">"
                                     "<TextInput id=\"in\" x=\"0\" y=\"0\" width=\"120\" height=\"40\" font=\"normal\" "
            + inputAttributes + "/></customView></doc>";

        REQUIRE(manager.BuildFromXMLString(document) != -1);
        manager.InitializationFinished();
        manager.SetActiveScreen("home", 0);

        auto* input = static_cast<TextInput*>(manager.FindElementInTheActiveScreenById("in"));
        REQUIRE(input != nullptr);

        if(focus) {
            manager.SetActiveInput(input);
        }

        manager.Draw();
        return input;
    }

    bool IsCursorPixel(size_t index)
    {
        uint32_t pixel;
        std::memcpy(&pixel, &framebuffer[index * 4], sizeof(pixel));
        return (pixel & 0x00ffffffu) == CURSOR_RGB;
    }

    // Both buffers are scanned, because grvl renders into the back one and the
    // frame buffer is cleared before every render.
    size_t CountCursorPixels()
    {
        size_t count = 0;
        for(size_t i = 0; i < framebuffer.size() / 4; ++i) {
            if(IsCursorPixel(i)) {
                ++count;
            }
        }
        return count;
    }

    std::set<uint32_t> CursorColumns()
    {
        std::set<uint32_t> columns;
        for(size_t i = 0; i < framebuffer.size() / 4; ++i) {
            if(IsCursorPixel(i)) {
                columns.insert((i % (WIDTH * HEIGHT)) % WIDTH);
            }
        }
        return columns;
    }

    int32_t CursorColumn()
    {
        const std::set<uint32_t> columns = CursorColumns();
        return columns.empty() ? -1 : static_cast<int32_t>(*columns.begin());
    }

} // namespace

TEST_CASE("TextInput cursor: off by default", "[component][cursor]")
{
    Render(R"(text="ab" textColor="#ffffffff")", true);
    REQUIRE(CountCursorPixels() == 0);
}

TEST_CASE("TextInput cursor: drawn only while focused", "[component][cursor]")
{
    Render(R"(text="ab" cursor="true" cursorColor="#ff00ff00")", false);
    REQUIRE(CountCursorPixels() == 0);

    Render(R"(text="ab" cursor="true" cursorColor="#ff00ff00")", true);
    REQUIRE(CountCursorPixels() > 0);
}

TEST_CASE("TextInput cursor: one pixel wide, font tall", "[component][cursor]")
{
    TextInput* input = Render(R"(text="ab" cursor="true" cursorColor="#ff00ff00")", true);

    const size_t expected = input->GetButtonFont()->GetFontHeight() + 2 * TextInput::CursorOverhang(input->GetButtonFont()->GetFontHeight());

    REQUIRE(CursorColumns().size() == 1);
    REQUIRE(CountCursorPixels() == expected);
}

TEST_CASE("TextInput cursor: sits after the typed text", "[component][cursor]")
{
    Render(R"(cursor="true" cursorColor="#ff00ff00")", true);
    const int32_t empty = CursorColumn();

    Render(R"(text="abc" cursor="true" cursorColor="#ff00ff00")", true);
    const int32_t typed = CursorColumn();

    REQUIRE(empty >= 0);
    REQUIRE(typed > empty);
}

TEST_CASE("TextInput cursor: placeholder does not push the cursor", "[component][cursor]")
{
    Render(R"(cursor="true" cursorColor="#ff00ff00")", true);
    const int32_t without = CursorColumn();

    Render(R"(basicText="type here" cursor="true" cursorColor="#ff00ff00")", true);

    REQUIRE(without >= 0);
    REQUIRE(CursorColumn() == without);
}

TEST_CASE("TextInput cursor: keeps a gap from the text", "[component][cursor]")
{
    Render(R"(text="mm" cursor="true" cursorColor="#ff00ff00" textColor="#ffffffff")", true);

    int32_t lastTextColumn = -1;
    for(size_t i = 0; i < framebuffer.size() / 4; ++i) {
        uint32_t pixel;
        std::memcpy(&pixel, &framebuffer[i * 4], sizeof(pixel));
        pixel &= 0x00ffffffu;
        if(pixel != 0 && pixel != CURSOR_RGB) {
            lastTextColumn = std::max(lastTextColumn, static_cast<int32_t>((i % (WIDTH * HEIGHT)) % WIDTH));
        }
    }

    REQUIRE(lastTextColumn >= 0);
    REQUIRE(CursorColumn() - lastTextColumn > TextInput::CursorGap(FONT_HEIGHT));
}

TEST_CASE("TextInput cursor: stays inside the field when the text overflows", "[component][cursor]")
{
    Render(R"(text="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" cursor="true" cursorColor="#ff00ff00")", true);

    const int32_t column = CursorColumn();
    REQUIRE(column >= 0);
    REQUIRE(column < static_cast<int32_t>(WIDTH));
}

TEST_CASE("TextInput cursor: metrics scale with the font", "[component][cursor]")
{
    REQUIRE(TextInput::CursorGap(8) == 2);
    REQUIRE(TextInput::CursorOverhang(8) == 2);
    REQUIRE(TextInput::CursorGap(26) == 6);
    REQUIRE(TextInput::CursorOverhang(26) == 6);

    // A tiny font still gets a visible gap.
    REQUIRE(TextInput::CursorGap(3) >= 1);
    REQUIRE(TextInput::CursorOverhang(1) >= 1);
}

TEST_CASE("TextInput cursor: a blinking caret keeps asking for frames", "[component][cursor]")
{
    fakeClock = 0;
    Render(R"(text="ab" cursor="true" cursorColor="#ff00ff00")", true);
    REQUIRE(Manager::GetInstance().NeedsFrames());

    Render(R"(text="ab" cursor="true" cursorBlink="false" cursorColor="#ff00ff00")", true);
    REQUIRE_FALSE(Manager::GetInstance().NeedsFrames());
}

TEST_CASE("TextInput cursor: blinks on and off", "[component][cursor]")
{
    fakeClock = 0;
    Render(R"(text="ab" cursor="true" cursorColor="#ff00ff00")", true);
    REQUIRE(CountCursorPixels() > 0);

    // The cycle starts when the field takes focus, so advancing the clock
    // between renders would also restart it; redraw the same manager instead.
    Manager& manager = Manager::GetInstance();

    fakeClock = TextInput::CURSOR_BLINK_PERIOD_MS + 10;
    framebuffer.assign(framebuffer.size(), 0);
    manager.Draw();
    REQUIRE(CountCursorPixels() == 0);

    fakeClock = 2 * TextInput::CURSOR_BLINK_PERIOD_MS + 10;
    framebuffer.assign(framebuffer.size(), 0);
    manager.Draw();
    REQUIRE(CountCursorPixels() > 0);
}

TEST_CASE("TextInput cursor: cursorBlink=false stays lit", "[component][cursor]")
{
    fakeClock = 0;
    Render(R"(text="ab" cursor="true" cursorBlink="false" cursorColor="#ff00ff00")", true);
    Manager& manager = Manager::GetInstance();

    for(uint64_t t : { 0ull, 600ull, 1200ull, 1800ull }) {
        fakeClock = t;
        framebuffer.assign(framebuffer.size(), 0);
        manager.Draw();
        REQUIRE(CountCursorPixels() > 0);
    }
}

TEST_CASE("TextInput cursor: typing restarts the blink cycle", "[component][cursor]")
{
    fakeClock = 0;
    TextInput* input = Render(R"(text="ab" cursor="true" cursorColor="#ff00ff00")", true);
    Manager& manager = Manager::GetInstance();

    fakeClock = TextInput::CURSOR_BLINK_PERIOD_MS + 10;
    framebuffer.assign(framebuffer.size(), 0);
    manager.Draw();
    REQUIRE(CountCursorPixels() == 0); // in the dark half of the cycle

    input->AddCharacter('c');
    framebuffer.assign(framebuffer.size(), 0);
    manager.Draw();
    REQUIRE(CountCursorPixels() > 0); // typing made it visible again
}

TEST_CASE("TextInput cursor: clone carries the settings", "[component][cursor]")
{
    TextInput* input = Render(R"(cursor="true" cursorColor="#ff00ff00")", true);

    TextInput* copy = static_cast<TextInput*>(input->Clone());
    REQUIRE(copy->IsCursorEnabled());
    REQUIRE(copy->GetCursorColor() == input->GetCursorColor());
    REQUIRE(copy->IsCursorBlinking() == input->IsCursorBlinking());
    delete copy;
}
