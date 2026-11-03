
#include <catch2/catch_test_macros.hpp>

#include <grvl/ImageContent.h>
#include <grvl/grvl.h>

using namespace grvl;

TEST_CASE("ImageContent Read/Write", "[image]")
{

    gui_callbacks_t callbacks {};
    grvl::grvl::Init(&callbacks);

    ImageContent foo { 4, 4, 2, Format::ARGB8888 };

    foo.Fill(0x00, 0);
    foo.Fill(0xAB, 1);

    foo.WritePixel(0, 0, 0, 0x42);
    foo.WritePixel(1, 1, 0, 0x21);
    foo.WritePixel(3, 3, 0, 0x37);
    foo.WritePixel(3, 3, 1, 0xAA);
    foo.WritePixel(0, 3, 1, 0xBB);

    REQUIRE(foo.ReadPixel(2, 2, 0) == 0x00);
    REQUIRE(foo.ReadPixel(2, 2, 1) == 0xABABABAB);

    REQUIRE(foo.ReadPixel(0, 0, 0) == 0x42);
    REQUIRE(foo.ReadPixel(1, 1, 0) == 0x21);
    REQUIRE(foo.ReadPixel(3, 3, 0) == 0x37);
    REQUIRE(foo.ReadPixel(3, 3, 1) == 0xAA);
    REQUIRE(foo.ReadPixel(0, 3, 1) == 0xBB);

    grvl::grvl::Destroy();
}

TEST_CASE("ImageContent Blit", "[image]")
{

    gui_callbacks_t callbacks {};
    grvl::grvl::Init(&callbacks);

    ImageContent foo { 4, 4, 2, Format::ARGB8888 };
    ImageContent bar { 4, 4, 1, Format::ARGB8888 };

    foo.Fill(0xAA, 0);
    foo.Fill(0xBB, 1);
    bar.Fill(0x00, 0);

    // sanity checks
    REQUIRE(bar.ReadPixel(0, 0, 0) == 0x00000000);
    REQUIRE(foo.ReadPixel(0, 0, 0) == 0xAAAAAAAA);
    REQUIRE(foo.ReadPixel(0, 0, 1) == 0xBBBBBBBB);

    // copy frame 1 of foo into bar at (1, 1), some pixels truncated
    bar.Blit(1, 1, 0, &foo, 1);

    REQUIRE(bar.ReadPixel(0, 0, 0) == 0x00000000);
    REQUIRE(bar.ReadPixel(0, 3, 0) == 0x00000000);
    REQUIRE(bar.ReadPixel(3, 0, 0) == 0x00000000);
    REQUIRE(bar.ReadPixel(1, 1, 0) == 0xBBBBBBBB);
    REQUIRE(bar.ReadPixel(2, 2, 0) == 0xBBBBBBBB);
    REQUIRE(bar.ReadPixel(3, 3, 0) == 0xBBBBBBBB);

    // copy frame 0 of foo into bar at (-2, -2), some pixels truncated
    bar.Blit(-2, -2, 0, &foo, 0);

    REQUIRE(bar.ReadPixel(0, 0, 0) == 0xAAAAAAAA);
    REQUIRE(bar.ReadPixel(0, 3, 0) == 0x00000000);
    REQUIRE(bar.ReadPixel(3, 0, 0) == 0x00000000);
    REQUIRE(bar.ReadPixel(1, 1, 0) == 0xAAAAAAAA);
    REQUIRE(bar.ReadPixel(2, 2, 0) == 0xBBBBBBBB);
    REQUIRE(bar.ReadPixel(3, 3, 0) == 0xBBBBBBBB);

    grvl::grvl::Destroy();
}

TEST_CASE("ImageContent Resize", "[image]")
{

    gui_callbacks_t callbacks {};
    grvl::grvl::Init(&callbacks);

    ImageContent foo { 4, 4, 2, Format::RGB565 };

    foo.Fill(0xFF, 0);
    foo.Fill(0x00, 1);

    ImageContent* bar = foo.Resize(8, 8);

    REQUIRE(bar->GetColorFormat() == Format::RGB565);
    REQUIRE(bar->GetWidth() == 8);
    REQUIRE(bar->GetHeight() == 8);

    REQUIRE(bar->ReadPixel(0, 0, 0) == 0xFFFFFFFF);
    REQUIRE(bar->ReadPixel(3, 3, 0) == 0xFFFFFFFF);
    REQUIRE(bar->ReadPixel(0, 0, 1) == 0xFF000000);
    REQUIRE(bar->ReadPixel(3, 3, 1) == 0xFF000000);

    delete bar;

    grvl::grvl::Destroy();
}
