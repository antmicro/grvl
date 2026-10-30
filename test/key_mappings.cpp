#include <catch2/catch_test_macros.hpp>

#include <grvl/Manager.h>
#include <grvl/platform/KeyMappings.h>

#if defined(GRVL_LINUX_DESKTOP_SUPPORT)
#include <SDL2/SDL_keycode.h>
#endif
#if defined(GRVL_LINUX_NATIVE_SUPPORT)
#include <linux/input-event-codes.h>
#endif

namespace {
    uint64_t now;
    std::vector<std::string> events;

    void Record(void*, const grvl::Event::ArgVector& args)
    {
        events.push_back(args.at(0));
    }

    struct KeyboardFixture {
        KeyboardFixture()
        {
            now = 10000;
            events.clear();
            grvl::gui_callbacks_t callbacks {};
            callbacks.get_timestamp = [] { return now; };
            grvl::grvl::Init(&callbacks);
            grvl::Manager::Initialize(50, 50, 4, false);
            manager().AddCallbackToContainer("Record", Record);
        }

        ~KeyboardFixture() { grvl::grvl::Destroy(); }

        grvl::Manager& manager() { return grvl::Manager::GetInstance(); }

        void Load(const std::string& xml)
        {
            REQUIRE(manager().BuildFromXMLString(xml) == 0);
            manager().InitializationFinished();
            manager().SetActiveScreen("home", 0);
            manager().MainLoopIteration();
        }

        void Key(bool pressed, uint32_t code, uint32_t legacyCode = 0)
        {
            manager().ProcessKeyInput(pressed, code, legacyCode);
            manager().MainLoopIteration();
        }
    };

#if defined(GRVL_LINUX_DESKTOP_SUPPORT) || defined(GRVL_LINUX_NATIVE_SUPPORT)
    struct Mapping {
        uint32_t code;
        const char* id;
    };

    void CheckDefaults(void (*install)(grvl::Manager&), const std::vector<Mapping>& mappings, bool legacySDL = false)
    {
        KeyboardFixture fixture;
        install(fixture.manager());
        std::string xml = "<doc><customView id='home'>";
        for(const auto& mapping : mappings) {
            xml += "<key id='" + std::string(mapping.id) + "' onPress=\"Record('" + mapping.id
                + "')\" onRelease=\"Record('release')\" onLongPress=\"Record('long')\" onLongPressRepeat=\"Record('repeat')\"/>";
        }
        fixture.Load(xml + "</customView></doc>");
        for(const auto& mapping : mappings) {
            INFO(mapping.id);
            events.clear();
            const uint32_t legacyCode = legacySDL ? static_cast<uint16_t>(mapping.code) : 0;
            fixture.Key(true, mapping.code, legacyCode);
            REQUIRE(events == std::vector<std::string> { mapping.id });
            fixture.Key(false, mapping.code, legacyCode);
            REQUIRE(events == std::vector<std::string> { mapping.id, "release" });
        }
        // Backend defaults use the same 100ms repeat interval as XML defaults.
        events.clear();
        fixture.Key(true, mappings.front().code);
        now += 501;
        fixture.manager().MainLoopIteration();
        REQUIRE(events.back() == "long");
        now += 101;
        fixture.manager().MainLoopIteration();
        REQUIRE(events.back() == "repeat");
        fixture.Key(false, mappings.front().code);
    }
#endif
}

TEST_CASE("XML overrides key codes, preserves defaults and adds aliases", "[keyboard]")
{
    KeyboardFixture fixture;
    fixture.manager().AddDefaultKeyMappingToContainer(14, "backspace", 100);
    fixture.manager().AddDefaultKeyMappingToContainer(1, "escape", 100);
    fixture.Load(R"XML(
        <doc>
            <customView id="home">
                <key id="backspace" onPress="Record('backspace')"/>
                <key id="escape" onPress="Record('escape')"/>
                <key id="custom" onPress="Record('custom')" onLongPress="Record('long')" onLongPressRepeat="Record('repeat')"/>
            </customView>
            <keypadMapping>
                <key id="custom" code="14" repeat="250"/>
                <key id="escape" code="99"/>
            </keypadMapping>
        </doc>
    )XML");
    fixture.Key(true, 14);
    REQUIRE(events == std::vector<std::string> { "custom" });
    now += 501;
    fixture.manager().MainLoopIteration();
    REQUIRE(events.back() == "long");
    auto count = events.size();
    now += 101;
    fixture.manager().MainLoopIteration();
    REQUIRE(events.size() == count);
    now += 150;
    fixture.manager().MainLoopIteration();
    REQUIRE(events.back() == "repeat");
    fixture.Key(false, 14);
    events.clear();
    fixture.Key(true, 1);
    fixture.Key(false, 1);
    fixture.Key(true, 99);
    REQUIRE(events == std::vector<std::string> { "escape", "escape" });
}

TEST_CASE("Full-width key codes and legacy modifier overrides", "[keyboard]")
{
    KeyboardFixture fixture;
    fixture.manager().AddKeyMappingToContainer(0x40000050, "left", 100);
    fixture.manager().AddKeyMappingToContainer(80, "other", 100);
    fixture.manager().AddKeyMappingToContainer(42, "leftshift", 100);
    fixture.Load(R"XML(
        <doc>
            <customView id="home">
                <key id="left" onPress="Record('left')" onRelease="Record('release')"/>
                <key id="other" onPress="Record('other')"/>
                <key id="custom" onPress="Record('custom')"/>
            </customView>
            <keypadMapping>
                <key id="custom" code="1073742928"/>
            </keypadMapping>
        </doc>
    )XML");
    // An unhandled modifier must not capture the active key slot.
    fixture.Key(true, 42 | 0x1000);
    fixture.Key(true, 0x40000050 | 0x1000);
    REQUIRE(events == std::vector<std::string> { "left" });
    // Unrelated releases must not release the active key.
    fixture.Key(false, 42);
    REQUIRE(events.size() == 1);
    fixture.Key(false, 0x40000050);
    REQUIRE(events.back() == "release");
    fixture.Key(true, 80);
    fixture.Key(false, 80);
    fixture.Key(true, 0x40000050 | 0x400);
    REQUIRE(events.back() == "custom");
}

TEST_CASE("Legacy SDL XML bindings override defaults but not full-width overrides", "[keyboard]")
{
    KeyboardFixture fixture;
    fixture.manager().AddDefaultKeyMappingToContainer(0x40000050, "left", 100);
    fixture.Load(R"XML(
        <doc>
            <customView id="home">
                <key id="left" onPress="Record('left')"/>
                <key id="legacy" onPress="Record('legacy')" onRelease="Record('release')"/>
                <key id="custom" onPress="Record('custom')"/>
            </customView>
            <keypadMapping><key id="legacy" code="80"/></keypadMapping>
        </doc>
    )XML");
    fixture.Key(true, 0x40000050, 80);
    fixture.Key(false, 0x40000050, 80);
    REQUIRE(events == std::vector<std::string> { "legacy", "release" });
    fixture.manager().AddKeyMappingToContainer(0x40000050, "custom", 100);
    // Installing defaults again must not replace explicit application/XML bindings.
    fixture.manager().AddDefaultKeyMappingToContainer(0x40000050, "left", 100);
    fixture.Key(true, 0x40000050, 80);
    REQUIRE(events.back() == "custom");
}

#if defined(GRVL_LINUX_DESKTOP_SUPPORT)
TEST_CASE("SDL standard keyboard defaults", "[keyboard][sdl]")
{
    static_assert(SDLK_BACKSPACE == 8);
    static_assert(SDLK_ESCAPE == 27);
    const std::vector<Mapping> mappings {
#define GRVL_KEY(id, sdl, evdev) { sdl, #id },
#include "../src/platform/KeyboardMappings.inc"
#undef GRVL_KEY
    };
    CheckDefaults(grvl::InstallLinuxDesktopKeyMappings, mappings, true);
}
#endif

#if defined(GRVL_LINUX_NATIVE_SUPPORT)
TEST_CASE("LinuxNative standard keyboard defaults", "[keyboard][linuxnative]")
{
    static_assert(KEY_BACKSPACE == 14);
    static_assert(KEY_ESC == 1);
    const std::vector<Mapping> mappings {
#define GRVL_KEY(id, sdl, evdev) { evdev, #id },
#include "../src/platform/KeyboardMappings.inc"
#undef GRVL_KEY
    };
    CheckDefaults(grvl::InstallLinuxNativeKeyMappings, mappings);
}
#endif
