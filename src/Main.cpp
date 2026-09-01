#include "LevelScript.h"
#include "Coop.h"
#include "Sound.h"
#include "Decompress.h"
#include "Memory.h"
#include "Segment2.h"
#include "BehaviorScript.h"
#include "cxxopts.hpp"

bool VerbosePrinting = false;
std::string ActorsExport = "none";
bool SoundExport = false;
bool TweakExport = false;
bool CollisionFix = false;
bool SkyboxExport = false;
bool TexturesExport = false;
bool BehaviorsExport = false;
bool TextExport = false;
SM64GameType GameType;
u32 FoundScriptEntry = 0;
bool ExportSegment0 = false;
bool IgnoreSegment0 = false;

const std::set<std::string> SM64ROMManagerVersions = {
    "v0.6.2",
    "v0.7",
    "v0.7.1",
    "v0.7.2",
    "v0.7.4",
    "v0.7.5",
    "v0.7.6",
    "v0.7.7",
    "v0.7.8",
    "v0.7.9",
    "v0.7.10",
    "v0.7.11",
    "v1.0",
    "v1.0.1",
    "v1.0.2",
    "v1.0.3",
    "v1.0.4",
    "v1.1.1",
    "v1.2",
    "v1.3",
    "v1.3.0.1",
    "v1.4",
    "v1.4.0.1",
    "v1.5",
    "v1.6",
    "v1.6.1",
    "v1.6.2",
    "v1.6.3",
    "v1.7",
    "v1.7.1",
    "v1.7.2",
    "v1.7.3",
    "v1.7.4",
    "v1.7.5",
    "v1.7.6",
    "v1.7.7",
    "v1.7.8",
    "v1.7.9",
    "v1.7.10",
    "v1.8",
    "v1.8.1",
    "v1.8.2",
    "v1.8.3",
    "v1.9",
    "v1.9.1",
    "v1.9.2",
    "v1.9.3",
    "v1.9.4",
    "v1.9.5",
    "v1.9.6",
    "v1.9.7",
    "v1.9.8",
    "v1.9.9",
    "v1.9.10",
    "v1.9.11",
    "v1.9.12",
    "v1.9.13",
    "v1.9.15",
    "v1.9.17",
    "v1.9.18",
    "v1.10.1",
    "v1.10.2",
    "v1.10.3",
    "v1.10.4",
    "v1.10.5",
    "v1.10.6",
    "v1.11",
    "v1.11.1",
    "v1.11.2",
    "v1.11.3",
    "v1.11.4",
    "v1.11.5",
    "v1.11.6",
    "v1.11.7",
    "v1.11.8",
    "v1.11.9",
    "v1.11.10",
    "v1.11.12",
    "v1.11.13",
    "v1.11.14.1",
    "v1.11.15",
    "v1.12.0.2",
    "v1.12.1",
    "v1.12.2",
    "v1.12.3",
    "v1.12.4",
    "v1.12.5",
    "v1.12.6",
    "v1.12.7",
    "v1.12.8",
    "v1.12.9",
    "v1.12.9.1",
    "v1.12.11",
    "v1.12.12",
    "v1.12.13.1",
    "v1.12.14",
    "v1.12.15",
    "v1.12.16.1",
    "v1.13.1",
    "v1.13.2",
    "v1.13.3",
    "v1.13.4",
    "v1.13.5",
    "v1.13.6",
    "v1.13.6.1",
    "v1.13.7",
    "v1.13.8",
    "v1.13.9",
    "v1.13.10",
    "v1.13.11",
    "v1.13.12",
    "v1.14",
    "v1.14.1",
    "v1.14.1.1",
    "v1.14.2",
    "v1.14.3",
    "v1.14.4",
    "v1.14.5",
    "v1.14.6",
    "v1.14.7",
    "v1.14.8",
    "v1.14.9",
    "v1.14.10",
    "v1.14.11",
    "v1.14.12",
    "v1.14.13",
    "v1.14.14",
    "v1.14.15.2",
};

int main(int argc, char** argv) {
    cxxopts::Options Options("ROMDemangler", "");
    Options.add_options()
        ("rom", "ROM file", cxxopts::value<std::string>())
        ("levels", "List of level IDs to export", cxxopts::value<std::vector<int>>())
        ("actors", "Which actors to export, vanilla|custom|all|none", cxxopts::value<std::string>())
        ("sound", "Export Sound data", cxxopts::value<bool>())
        ("tweaks", "Export Tweaks", cxxopts::value<bool>())
        ("ram", "The File/Memory to get Segment 0 from", cxxopts::value<std::string>())
        ("fix-collision", "Fix collision geometry", cxxopts::value<bool>())
        ("verbose", "Print more info", cxxopts::value<bool>())
        ("ignore-seg-0", "Don't export stuff from Segment 0", cxxopts::value<bool>())
        ("custom-symbols", "Path to a JSON symbol file that overrides entries from symbolMap.json (for romhacks with custom symbols)", cxxopts::value<std::string>())
        ("skyboxes", "Export Skyboxes", cxxopts::value<bool>())
        ("textures", "Export Segment2 textures", cxxopts::value<bool>())
        ("text", "Export dialogs and course names", cxxopts::value<bool>())
        ("behaviors", "Export behavior scripts", cxxopts::value<bool>())
        ("h,help", "Print usage");

    auto Result = Options.parse(argc, argv);
    SoundExport = Result["sound"].as<bool>();
    TweakExport = Result["tweaks"].as<bool>();
    VerbosePrinting = Result["verbose"].as<bool>();
    IgnoreSegment0 = Result["ignore-seg-0"].as<bool>();
    CollisionFix = Result["fix-collision"].as<bool>();
    SkyboxExport = Result["skyboxes"].as<bool>();
    TexturesExport = Result["textures"].as<bool>();
    BehaviorsExport = Result["behaviors"].as<bool>();
    TextExport = Result["text"].as<bool>();
    std::string RAMPath;
    std::string CustomSymbolsPath;
    if (Result.count("actors")) ActorsExport = Result["actors"].as<std::string>();
    if (Result.count("ram")) {
        RAMPath = Result["ram"].as<std::string>();
        ExportSegment0 = true;
    }
    if (Result.count("custom-symbols")) {
        CustomSymbolsPath = Result["custom-symbols"].as<std::string>();
    }

    if (Result.count("help") || !Result.count("rom") || !Result.count("levels")) {
        std::cout << Options.help() << std::endl;
        return 0;
    }

    std::string RomPath = Result["rom"].as<std::string>();
    std::vector<int> LvlIDs = Result["levels"].as<std::vector<int>>();

    N64Rom Rom;
    Rom.OpenFile(RomPath.c_str(), ExportSegment0 ? RAMPath.c_str() : nullptr);
    InitMemoryMap(CustomSymbolsPath);

    std::string GameTypeStr = "ROM is made with ";

    const u64 EditorPatterns[] = {
        0x800800000E0000C4,
        0x0800000A00A00078,
        0x800800001900001C
    };
    u64 PatternInRom = Rom.ReadBytesPhysical<u64>(EDITOR_MAGIC_ADDR);
    u64 PatternInRom2 = Rom.ReadBytesPhysical<u64>(EDITOR_MAGIC_ADDR+8);
    for (auto &P : EditorPatterns) {
        if (PatternInRom == P || PatternInRom2 == P) {
            GameType.SetID(GT_EDITOR);
            GameTypeStr += "SM64 Editor";
            break;
        }
    }
    
    if (GameType.GetID() != GT_EDITOR) {
        if (Rom.Size >= BBP_SIGNATURE_ADDR) {
            if (Rom.ReadBytesPhysical<u32>(BBP_SIGNATURE_ADDR) == BBP_SIGNATURE) {
                GameType.SetID(GT_BBP);
                GameTypeStr += "Bowser's Blueprints ";

                u32 BBPMetaDataAddr = Rom.ReadBytesPhysical<u32>(BBP_SIGNATURE_ADDR+4);
                s16 Major = Rom.ReadBytesPhysical<s16>(BBPMetaDataAddr);
                s16 Minor = Rom.ReadBytesPhysical<s16>(BBPMetaDataAddr+2);
                s16 Patch = Rom.ReadBytesPhysical<s16>(BBPMetaDataAddr+4);
                GameTypeStr += std::format("v{}.{}.{}", Major, Minor, Patch);
            }
        }

        if (GameType.GetID() != GT_BBP) {
            u32 VOffset = RM_VERSION_ADDR;
            s32 DevInfo = Rom.ReadBytesPhysical<s32>(VOffset);
            u8 Major = Rom.ReadBytesPhysical<u8>(VOffset+4);
            u8 Minor = Rom.ReadBytesPhysical<u8>(VOffset+5);
            u8 Build = Rom.ReadBytesPhysical<u8>(VOffset+6);
            u8 Revision = Rom.ReadBytesPhysical<u8>(VOffset+7);

            std::string Version = std::format("v{}.{}", Major, Minor);
            if (Build > 0) {
                Version += std::format(".{}", Build);
            }
            if (Revision > 0) {
                Version += std::format(".{}", Revision);
            }
            if (SM64ROMManagerVersions.count(Version)) {
                GameType.SetID(GT_ROM_MANAGER);
                GameTypeStr += "SM64 ROM Manager " + Version;
            }
        }
    }

    if (GameType.GetID() == GT_UNKNOWN) {
        const u8 Pattern[] = {
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x00, 0x00, 0x00, 0x00,
            0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x00, 0x00, 0x00, 0x00,
            0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
            0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
        };

        size_t PatternLen = sizeof(Pattern);
        u8 *Start = Rom.Data;
        u8 *End = Rom.Data + Rom.Size;
        u8 *Found = std::search(Start, End, Pattern, Pattern + PatternLen);
        if (Found != End) {
            GameTypeStr += "HackerSM64";
            GameType.SetID(GT_HACKER);
        } else {
            GameTypeStr += "Decomp";
            GameType.SetID(GT_DECOMP);
        }
    }
    printf("%s", (GameTypeStr + "\n").c_str());

    std::error_code ErrCode;
    if (fs::exists("output")) fs::remove_all("output", ErrCode);
    fs::create_directories("output");

    if (std::find(LvlIDs.begin(), LvlIDs.end(), 99) != LvlIDs.end()) {
        for (const auto &[LvlID, Name] : LevelNames) {
            if (LvlID == 25) continue;
            ExportLevel(Rom, LvlID);
        }
    } else {
        for (const int &LvlID : LvlIDs) {
            ExportLevel(Rom, LvlID);
        }
    }

    if (TexturesExport) ExportSeg2Textures(Rom);
    if (SoundExport) ExportSequences(Rom);
    ExportLua(Rom);
    printf("Export done!\n");
    Rom.CloseFile();
    return 0;
}