#include "Segment2.h"
#include "F3D.h"
#include "Memory.h"
#include "N64Rom.h"
#include "stb_image_write.h"
#include <format>
#include <set>

struct TransitionTexData {
    u32 Address = 0;
    const char *Name;
    u32 Width = 0;
    u32 Height = 0;
    u32 Size = 0;
};

struct Segment2Match {
    u32 Addr = 0, Code = 0, Size = 0;
};

const TransitionTexData TransitionData[] = {
    {0x122B8, "0F458", 32, 64, 0x800},
    {0x12AB8, "0FC58", 32, 64, 0x800},
    {0x132B8, "10458", 64, 64, 0x1000},
    {0x142B8, "11458", 32, 64, 0x800}
};

const std::string CourseNames[] = {
    "COURSE_NONE", "COURSE_BOB", "COURSE_WF", "COURSE_JRB", "COURSE_CCM", 
    "COURSE_BBH", "COURSE_HMC", "COURSE_LLL", "COURSE_SSL", "COURSE_DDD", 
    "COURSE_SL", "COURSE_WDW", "COURSE_TTM", "COURSE_THI", "COURSE_TTC", 
    "COURSE_RR", "COURSE_BITDW", "COURSE_BITFS", "COURSE_BITS", "COURSE_PSS", 
    "COURSE_COTMC", "COURSE_TOTWC", "COURSE_VCUTM", "COURSE_WMOTR", "COURSE_SA", 
    "COURSE_CAKE_END"
};

static std::string SM64ToString(u8 num) {
    if (num < 0x0A) {
        return std::string(1, (char)(num + 0x30));
    } else if (num < 0x24) {
        return std::string(1, (char)(num + 0x37));
    } else if (num < 0x3E) {
        return std::string(1, (char)(num + 0x3D));
    }

    switch (num) {
        case 62:   return "'";
        case 63:   return ".";
        case 80:   return "^";
        case 81:   return "|";
        case 82:   return "<";
        case 83:   return ">";
        case 84:   return "[A]";
        case 85:   return "[B]";
        case 86:   return "[C]";
        case 87:   return "[Z]";
        case 88:   return "[R]";
        case 111:  return ",";
        case 0x9E: return " ";
        case 0x9F: return "-";
        case 208:  return "/";
        case 209:  return "the";
        case 210:  return "you";
        case 224:  return "[%]";
        case 225:  return "(";
        case 226:  return ")(";
        case 227:  return ")";
        case 228:  return "↔";
        case 229:  return "&";
        case 230:  return ":";
        case 240:  return "゛";
        case 241:  return "゜";
        case 242:  return "!";
        case 243:  return "%";
        case 244:  return "?";
        case 245:  return "『";
        case 246:  return "』";
        case 247:  return "~";
        case 248:  return "…";
        case 249:  return "$";
        case 250:  return "★";
        case 251:  return "×";
        case 252:  return "・";
        case 253:  return "☆";
        case 254:  return "\\\n";
        default:   return "";
    }
}

void FindAndLoadSegment2(N64Rom &Rom) {
    u32 Seg2Start = 0;
    u32 Seg2End = 0;
    std::vector<std::pair<u32, u32>> Matches;
    std::vector<Segment2Match> CodeMatches;
    std::unordered_map<u32, u32> CompMap;

    for (u32 i = 0; i < Rom.Size - 16; i += 4) {
        u32 Magic = Rom.ReadBytesPhysical<u32>(i);
        if (Magic == 0x4D494F30 || Magic == 0x59617930 || Magic == 0x524E4301 || Magic == 0x524E4302) {
            u32 UncompSize = 0;
            if (Magic == 0x524E4301 || Magic == 0x524E4302) {
                for (int j = 0; j < 4; j++) {
                    UncompSize = (UncompSize << 8) | Rom.ReadBytesPhysical<u8>(i + 4 + j);
                }
            } else {
                UncompSize = Rom.ReadBytesPhysical<u32>(i + 4);
            }

            if (UncompSize >= 32768 && UncompSize <= 131072) {
                Matches.push_back({i, UncompSize});
                CompMap[i] = UncompSize;
            }
        }
    }

    u32 WCount = Rom.Size / 4;
    for (u32 I = 0; I < WCount; ++I) {
        u32 Word = Rom.ReadBytesPhysical<u32>(I * 4);
        if ((Word >> 26) != 15) continue;

        u32 LuiUpper = Word & 0xFFFF;
        
        u32 ScanStart = (I >= 20) ? (I - 20) : 0;
        u32 ScanEnd = MIN(WCount, I + 20);

        bool HasLower = false;
        bool HasSegLoad = false;
        u32 ReconstructedAddr = 0;

        for (u32 J = ScanStart; J < ScanEnd; ++J) {
            u32 ScanWord = Rom.ReadBytesPhysical<u32>(J * 4);
            u32 ScanOp = ScanWord >> 26;

            if (ScanOp == 9 || ScanOp == 13) {
                u32 Lower = ScanWord & 0xFFFF;
                
                u32 OriginalUpper = (Lower >= 0x8000) ? (LuiUpper - 1) & 0xFFFF : LuiUpper;
                u32 TestAddr = (OriginalUpper << 16) | Lower;

                if (CompMap.count(TestAddr)) {
                    HasLower = true;
                    ReconstructedAddr = TestAddr;
                }
            }

            if ((ScanWord & 0xFFE0FFFF) == 0x24000002 || (ScanWord & 0xFFE0FFFF) == 0x34000002) {
                HasSegLoad = true;
            }
        }

        if (HasLower && HasSegLoad) {
            u32 CodeAddr = I * 4;
            CodeMatches.push_back({ReconstructedAddr, CodeAddr, CompMap[ReconstructedAddr]});
        }
    }

    printf("Found Segment 2 references:\n");

    for (const auto &Match : CodeMatches) {
        printf("  0x%08x  (code: 0x%08x, size: 0x%x)\n", Match.Addr, Match.Code, Match.Size);
    }

    if (!CodeMatches.empty()) {
        Seg2Start = CodeMatches[0].Addr;
        Seg2End = Rom.Size;

        if (GameType.IsOldBinary()) {
            for (const auto &Match : Matches) {
                if (Match.first == 0x00800000) {
                    Seg2Start = 0x00800000 + 0x3156;
                    printf("Old binary Segment 2 detected at 0x00800000, using Segment 2 from 0x%08x\n", Seg2Start);
                    break;
                }
            }
        } else {
            printf("Selected: 0x%08x\n", Seg2Start);
        }
    } else {
        Seg2Start = 0x800000;
        Seg2End = Rom.Size;
    }

    LoadSegment(Rom, 0x02, Seg2Start, Seg2End, true);
}

void ExportSeg2Textures(N64Rom &Rom) {
    std::string Seg2Path = "output/textures/segment2/";
    fs::create_directories(Seg2Path);

    printf("Exporting textures\n");

    u32 BaseSegment2Addr = 0x02000000;
    u32 NameOffsets = 0;
    std::set<u32> OverrideAddrs = {0x2600, 0x3200, 0x3A00, 0x3C00, 0x3E00};
    for (u32 Address = 0; Address < 0x4A00; Address += 0x200) {        
        if (OverrideAddrs.count(Address)) {
            NameOffsets += 0x200;
        } else if (Address == 0x4200) {
            NameOffsets += 0xA00;
        }

        u32 PixelCount = 16 * 16;

        std::vector<u8> SrcData(0x200);
        for (u32 I = 0; I < 0x200; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + Address + I);
        std::vector<u8> RGBA(PixelCount * 4);
        BinImg::DecodeRGBA16(SrcData.data(), RGBA.data(), PixelCount);

        u32 Addr = Address + NameOffsets;
        std::string FilePath = Seg2Path + std::format("segment2.{:05X}.rgba16.png", Addr);
        stbi_write_png(FilePath.c_str(), 16, 16, 4, RGBA.data(), 16 * 4);
    }

    NameOffsets = 0xB50;
    for (u32 Address = 0x7000; Address < 0x7600; Address += 0x200) {
        u32 PixelCount = 16 * 16;

        std::vector<u8> SrcData(0x200);
        for (u32 I = 0; I < 0x200; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + Address + I);
        std::vector<u8> RGBA(PixelCount * 4);
        BinImg::DecodeRGBA16(SrcData.data(), RGBA.data(), PixelCount);

        u32 Addr = Address + NameOffsets;
        std::string FilePath = Seg2Path + std::format("segment2.{:05X}.rgba16.png", Addr);
        stbi_write_png(FilePath.c_str(), 16, 16, 4, RGBA.data(), 16 * 4);
    }

    for (u32 Address = 0x7600; Address < 0x7700; Address += 0x80) {
        u32 PixelCount = 8 * 8;

        std::vector<u8> SrcData(0x80);
        for (u32 I = 0; I < 0x80; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + Address + I);
        std::vector<u8> RGBA(PixelCount * 4);
        BinImg::DecodeRGBA16(SrcData.data(), RGBA.data(), PixelCount);

        u32 Addr = Address + NameOffsets;
        std::string FilePath = Seg2Path + std::format("segment2.{:05X}.rgba16.png", Addr);
        stbi_write_png(FilePath.c_str(), 8, 8, 4, RGBA.data(), 8 * 4);
    }

    for (u32 Address = 0x5900; Address < 0x7000; Address += 0x40) {
        u32 PixelCount = 16 * 8;

        std::vector<u8> SrcData(0x40);
        for (u32 I = 0; I < 0x40; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + Address + I);
        std::vector<u8> RGBA(PixelCount * 4);
        BinImg::DecodeIA4(SrcData.data(), RGBA.data(), PixelCount);

        std::string FilePath = Seg2Path + std::format("font_graphics.{:05X}.ia4.png", Address);
        stbi_write_png(FilePath.c_str(), 16, 8, 4, RGBA.data(), 16 * 4);
    }

    u32 CreditsOffsets = 0x6200 - 0x4A00;
    for (u32 Address = 0x4A00; Address < 0x5900; Address += 0x80) {
        u32 PixelCount = 8 * 8;

        std::vector<u8> SrcData(0x80);
        for (u32 I = 0; I < 0x80; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + Address + I);
        std::vector<u8> RGBA(PixelCount * 4);
        BinImg::DecodeRGBA16(SrcData.data(), RGBA.data(), PixelCount);

        u32 Addr = Address + CreditsOffsets;
        std::string FilePath = Seg2Path + std::format("segment2.{:05X}.rgba16.png", Addr);
        stbi_write_png(FilePath.c_str(), 8, 8, 4, RGBA.data(), 8 * 4);
    }

    const std::string ShadowNames[] = {"shadow_quarter_circle", "shadow_quarter_square"};
    for (u32 Char = 0; Char < 2; Char++) {
        u32 Address = Char * 0x100 + 0x120B8;
        u32 PixelCount = 16 * 16;

        std::vector<u8> SrcData(0x100);
        for (u32 I = 0; I < 0x100; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + Address + I);
        std::vector<u8> RGBA(PixelCount * 4);
        BinImg::DecodeIA8(SrcData.data(), RGBA.data(), PixelCount);

        std::string FilePath = Seg2Path + ShadowNames[Char] + ".ia8.png";
        stbi_write_png(FilePath.c_str(), 16, 16, 4, RGBA.data(), 16 * 4);
    }

    for (const auto &Warp : TransitionData) {
        u32 PixelCount = Warp.Width * Warp.Height;

        std::vector<u8> SrcData(Warp.Size);
        for (u32 I = 0; I < Warp.Size; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + Warp.Address + I);
        std::vector<u8> RGBA(PixelCount * 4);
        BinImg::DecodeIA8(SrcData.data(), RGBA.data(), PixelCount);

        std::string FilePath = Seg2Path + std::format("segment2.{}.ia8.png", Warp.Name);
        stbi_write_png(FilePath.c_str(), Warp.Width, Warp.Height, 4, RGBA.data(), Warp.Width * 4);
    }

    s32 WaterOffsets = 0x11C58 - 0x14AB8;
    for (u32 Tex = 0; Tex < 5; Tex++) {
        u32 TexLoc = Tex * 0x800 + 0x14AB8;
        u32 PixelCount = 32 * 32;

        std::vector<u8> SrcData(0x800);
        for (u32 I = 0; I < 0x800; I++) SrcData[I] = Rom.ReadBytes<u8>(BaseSegment2Addr + TexLoc + I);
        std::vector<u8> RGBA(PixelCount * 4);

        if (Tex == 3) {
            BinImg::DecodeIA16(SrcData.data(), RGBA.data(), PixelCount);

            std::string FilePath = Seg2Path + std::format("segment2.{:05X}.ia16.png", (s32)TexLoc + WaterOffsets);
            stbi_write_png(FilePath.c_str(), 32, 32, 4, RGBA.data(), 32 * 4);
        } else {
            BinImg::DecodeRGBA16(SrcData.data(), RGBA.data(), PixelCount);

            std::string FilePath = Seg2Path + std::format("segment2.{:05X}.rgba16.png", (s32)TexLoc + WaterOffsets);
            stbi_write_png(FilePath.c_str(), 32, 32, 4, RGBA.data(), 32 * 4);
        }
    }
}

std::string GetRomText(N64Rom &Rom) {
    std::ostringstream TextDump;

    u32 DialogTableAddr = 0x0200FFC8;
    for (u32 Dialog = 0; Dialog < 170; Dialog++) {
        u32 Entry = DialogTableAddr + (Dialog * 16);
        
        u32 Unused = Rom.ReadBytes<u32>(Entry);
        u8 LinesPerBox = Rom.ReadBytes<u8>(Entry + 4);
        s16 LeftOffset = Rom.ReadBytes<s16>(Entry + 6);
        s16 Width = Rom.ReadBytes<s16>(Entry + 8);
        u32 StrOffset = Rom.ReadBytes<u32>(Entry + 12);

        std::string Str = "";
        u32 Curr = StrOffset;
        while (true) {
            u8 StrNum = Rom.ReadBytes<u8>(Curr++);
            if (StrNum != 0xFF) {
                Str += SM64ToString(StrNum);
            } else {
                break;
            }
        }

        TextDump << std::format("smlua_text_utils_dialog_replace(DIALOG_{:03d}, {}, {}, {}, {},\n\"{}\"\n)\n\n", 
                            Dialog, Unused, LinesPerBox, LeftOffset, Width, Str);
    }

    u32 LevelNamesAddr = 0x008140BE;
    u32 ActTableAddr = 0x00814A82;
    for (u32 Course = 0; Course < 26; Course++) {
        u32 NamePtr = Rom.ReadBytes<u32>(LevelNamesAddr + Course * 4);
        std::string Str = "";
        u32 Curr = NamePtr;
        while (true) {
            u8 StrNum = Rom.ReadBytes<u8>(Curr++);
            if (StrNum != 0xFF) {
                Str += SM64ToString(StrNum);
            } else {
                break;
            }
        }

        if (Course < 15) {
            std::vector<std::string> Acts;
            for (u32 Act = 0; Act < 6; Act++) {
                u32 ActPtr = Rom.ReadBytes<u32>(ActTableAddr + Course * 24 + Act * 4);
                std::string ActStr = "";
                u32 Curr = ActPtr;
                while (true) {
                    u8 StrNum = Rom.ReadBytes<u8>(Curr++);
                    if (StrNum != 0xFF) {
                        ActStr += SM64ToString(StrNum);
                    } else {
                        break;
                    }
                }
                Acts.push_back(ActStr);
            }
            
            TextDump << std::format("smlua_text_utils_course_acts_replace({}, \"{}\", \"{}\", \"{}\", \"{}\", \"{}\", \"{}\", \"{}\")\n\n",
                                CourseNames[Course+1], Str, Acts[0], Acts[1], Acts[2], Acts[3], Acts[4], Acts[5]);
        } else if (Course < 25) {
            TextDump << std::format("smlua_text_utils_secret_star_replace({} + 1, \"{}\")\n", Course, Str);
        } else {
            TextDump << std::format("smlua_text_utils_castle_secret_stars_replace(\"{}\")\n", Str);
        }
    }

    u32 Extra = ActTableAddr + (15 * 6 * 4);
    for (u32 I = 0; I < 7; I++) {
        u32 ExPtr = Rom.ReadBytes<u32>(Extra + I * 4);
        std::string Str = "";
        u32 Curr = ExPtr;
        while (true) {
            u8 StrNum = Rom.ReadBytes<u8>(Curr++);
            if (StrNum != 0xFF) {
                Str += SM64ToString(StrNum);
            } else {
                break;
            }
        }
        TextDump << std::format("smlua_text_utils_extra_text_replace({}, \"{}\")\n", I, Str);
    }

    return TextDump.str();
}