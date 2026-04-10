struct Header {
    char magic[8];
    be u32 version;
    be s32 height;
    be u64 entries;
    be u64 kernings;
};

struct Entry {
    be u32 bitmap;
    be u32 unicode;
    be s16 width;
    be s16 height;
    be s8 xoff;
    be s8 yoff;
    be s16 advance;
    u8 image[width * height] @ bitmap;
};

struct Kerning {
    be u32 prev;
    be u32 next;
    be s32 horizontal;
};

struct File {
    Header header;
    Entry entries[header.entries];
    Kerning kernings[header.kernings];
};

File file @ 0x00;
