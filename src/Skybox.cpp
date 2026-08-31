#include "Skybox.h"
#include "Memory.h"
#include "stb_image_write.h"
#include "BinImg.h"
#include "Main.h"

struct SkyboxTile {
    std::vector<u8> Texture;
    std::string Name;
};

static std::vector<SkyboxTile> GetSkyboxTiles(u32 SegAddr) {
    const u32 Bank = SegAddr >> 24;
    const auto &SegData = SegmentData[Bank];
    const s32 NumTiles = (SegData.size() - 0x140) / 2048;

    printf("Num skybox tiles: %i\n", NumTiles);

    std::vector<SkyboxTile> Tiles;
    Tiles.reserve(NumTiles);

    for (int i = 0; i < NumTiles; i++) {
        u32 Offset = i * 2048;

        SkyboxTile Tile;
        Tile.Name = std::format("SkyboxCustom_{:08X}_{}", SegmentedToROM(SegAddr + Offset), i);
        Tile.Texture.resize(4096);

        BinImg::DecodeRGBA16(SegData.data() + Offset, Tile.Texture.data(), 1024);

        Tiles.push_back(std::move(Tile));
    }

    return Tiles;
}

static void ExportSkyboxTiles(const std::vector<SkyboxTile> &Tiles) {
    std::string Path = "output/levels/textures/skybox_tiles/";
    fs::create_directories(Path);

    for (const auto &Tile : Tiles) {
        std::string FileName = Path + Tile.Name + ".rgba16.png";
        stbi_write_png(FileName.c_str(), 32, 32, 4, Tile.Texture.data(), 32 * 4);
    }
}

static void ExportPtrList(FILE* File, const std::vector<SkyboxTile> &Tiles, const std::string &SkyboxName) {
    for (const auto &Tile : Tiles) {
        fprintf(File,
            "ALIGNED8 static const Texture %s[] = \"%s\";\n\n",
            Tile.Name.c_str(),
            ("textures/skybox_tiles/" + Tile.Name + ".rgba16").c_str()
        );
    }

    fprintf(File, "const Texture *const %s[] = {\n", SkyboxName.c_str());

    int Rows = Tiles.size() / 8;
    int TotalTiles = Rows * 8 + Rows * 2;

    for (int y = 0; y < Rows; y++) {
        fprintf(File, "\t%s,\n", Tiles[y * 8 + 7].Name.c_str());

        for (int x = 0; x < 8; x++) {
            fprintf(File, "\t%s,\n", Tiles[y * 8 + x].Name.c_str());
        }

        fprintf(File, "\t%s,\n", Tiles[y * 8 + 0].Name.c_str());
    }

    for (int i = TotalTiles; i < 80; i++) {
        fprintf(File, "\t%s,\n", Tiles.back().Name.c_str());
    }

    fprintf(File, "};\n");
}

bool ExportSkybox(LevelScript &Script, std::string &SkyboxName) {
    u32 SegAddr = 0x0A000000;

    if (!SkyboxExport || !ValidateMemAddr(SegAddr)) {
        return false;
    }

    SkyboxName = std::format("SkyboxCustom_{:08X}", SegmentedToROM(SegAddr));

    printf("Exporting skybox %s\n", SkyboxName.c_str());

    std::vector<SkyboxTile> Tiles = GetSkyboxTiles(SegAddr);

    if (Tiles.empty()) {
        return false;
    }

    ExportSkyboxTiles(Tiles);

    std::string LevelPath = "output/levels/" + Script.Name + "/";
    std::string SkyboxPath = LevelPath + "skybox.inc.c";

    fs::create_directories(LevelPath);

    FILE* File = fopen(SkyboxPath.c_str(), "w");

    if (!File) {
        printf("Can't open \"%s\"\n", SkyboxPath.c_str());
        return false;
    }

    ExportPtrList(File, Tiles, SkyboxName);

    fclose(File);

    return true;
}