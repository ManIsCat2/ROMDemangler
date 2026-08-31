#include "Model.h"
#include "LevelScript.h"
#include "Actor.h"
#include "Memory.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const u8 G_VTX_F3DEX2            = 0x01;
const u8 G_TRI1_F3DEX2           = 0x05;
const u8 G_TRI2_F3DEX2           = 0x06;
const u8 G_TEXTURE_F3DEX2        = 0xD7;
const u8 G_GEOMETRYMODE_F3DEX2   = 0xD9;
const u8 G_MOVEMEM_F3DEX2        = 0xDC;
const u8 G_DL_F3DEX2             = 0xDE;
const u8 G_ENDDL_F3DEX2          = 0xDF;
const u8 G_SETOTHERMODE_H_F3DEX2 = 0xE3;
const u8 G_SETOTHERMODE_L_F3DEX2 = 0xE2;

const u8 G_TRI2_F3DEX            = (u8)(G_IMMFIRST - 14);

const char *F3D_CC(enum F3DCCPart Part, u16 Element) {
    if (Part == CC_PART_A) {
        switch (Element) {
            case 0x0: return "COMBINED";
            case 0x1: return "TEXEL0";
            case 0x2: return "TEXEL1";
            case 0x3: return "PRIMITIVE";
            case 0x4: return "SHADE";
            case 0x5: return "ENVIRONMENT";
            case 0x6: return "1";
            case 0x7: return "NOISE";
            default:  return "0";
        }
    } else if (Part == CC_PART_B) {
        switch (Element) {
            case 0x0: return "COMBINED";
            case 0x1: return "TEXEL0";
            case 0x2: return "TEXEL1";
            case 0x3: return "PRIMITIVE";
            case 0x4: return "SHADE";
            case 0x5: return "ENVIRONMENT";
            case 0x6: return "CENTER";
            case 0x7: return "K4";
            default:  return "0";
        }
    } else if (Part == CC_PART_C) {
        switch (Element) {
            case 0x0: return "COMBINED";
            case 0x1: return "TEXEL0";
            case 0x2: return "TEXEL1";
            case 0x3: return "PRIMITIVE";
            case 0x4: return "SHADE";
            case 0x5: return "ENVIRONMENT";
            case 0x6: return "SCALE";
            case 0x7: return "COMBINED_ALPHA";
            case 0x8: return "TEXEL0_ALPHA";
            case 0x9: return "TEXEL1_ALPHA";
            case 0xA: return "PRIMITIVE_ALPHA";
            case 0xB: return "SHADE_ALPHA";
            case 0xC: return "ENV_ALPHA";
            case 0xD: return "LOD_FRACTION";
            case 0xE: return "PRIM_LOD_FRAC";
            case 0xF: return "K5";
            default:  return "0";
        }
    } else if (Part == CC_PART_D) {
        switch (Element) {
            case 0x0: return "COMBINED";
            case 0x1: return "TEXEL0";
            case 0x2: return "TEXEL1";
            case 0x3: return "PRIMITIVE";
            case 0x4: return "SHADE";
            case 0x5: return "ENVIRONMENT";
            case 0x6: return "1";
            default:  return "0";
        }
    }
    return "0";
}

const char *F3D_AC(enum F3DCCPart Part, u16 Element) {
    if (Part == CC_PART_A || Part == CC_PART_B || Part == CC_PART_D) {
        switch (Element) {
            case 0x0: return "COMBINED";
            case 0x1: return "TEXEL0";
            case 0x2: return "TEXEL1";
            case 0x3: return "PRIMITIVE";
            case 0x4: return "SHADE";
            case 0x5: return "ENVIRONMENT";
            case 0x6: return "1";
            default:  return "0";
        }
    } else if (Part == CC_PART_C) {
        switch (Element) {
            case 0x0: return "LOD_FRACTION";
            case 0x1: return "TEXEL0";
            case 0x2: return "TEXEL1";
            case 0x3: return "PRIMITIVE";
            case 0x4: return "SHADE";
            case 0x5: return "ENVIRONMENT";
            case 0x6: return "PRIM_LOD_FRAC";
            default:  return "0";
        }
    }
    return "0";
}

F3DTexture &GetCurrTex(std::vector<F3DTexture> &Textures) {
    if (Textures.empty()) {
        F3DTexture Tex = {};
        Tex.TextureSeg = 0;
        Tex.Texture = 0;
        Tex.Tile = 0xFF;
        Tex.ImgType = F3D_IMG_RGBA;
        Tex.BitDepth = 16;
        Tex.Length = 0;
        Tex.Width = 0;
        Tex.Height = 0;
        Tex.Palette = 0;
        Tex.PaletteSeg = 0;
        Textures.push_back(Tex);
    }
    return Textures.back();
}

inline void PushActiveTextures(std::vector<F3DTexture> &Textures) {
    if (!Textures.empty()) {
        F3DTexture &Cur = Textures.back();
        if (Cur.Texture != 0) {
            F3DTexture Cpy = Cur;
            Cpy.Texture = 0;
            Cpy.TextureSeg = 0;
            Cpy.Length = 0;
            Textures.push_back(Cpy);
        }
    }
}

void ParseRDPCommands(std::vector<F3DTexture> &Textures, u32 W0, u32 W1, u8 Cmd, bool Write, bool IsActor = false, Actor *Act = nullptr, FILE *ModelDump = nullptr, std::string LvlName="", u8 Area=0) {
    static u32 CurrentLoadBitDepth = 16;

    if (!Write) {
        auto GetBitDepthFromSize = [](u32 size) {
            switch (size) {
                case G_IM_SIZ_4b:  return 4;
                case G_IM_SIZ_8b:  return 8;
                case G_IM_SIZ_16b: return 16;
                case G_IM_SIZ_32b: return 32;
                default:           return 16;
            }
        };

        switch (Cmd) {
            case G_SETTIMG: {
                // if we already have tex, dont override
                // hopefully this doesnt break aynthing :pray:
                if (!Textures.empty()) {
                    F3DTexture &Cur = Textures.back();
                    if (Cur.Texture != 0 && Cur.Texture != W1 && Cur.Texture != Cur.Palette) {
                        PushActiveTextures(Textures);
                    }
                }

                u32 ImgType = C0(21, 3);
                u32 BitDepth = GetBitDepthFromSize(C0(19, 2));

                CurrentLoadBitDepth = BitDepth;

                F3DTexture &Tex = GetCurrTex(Textures);
                Tex.TextureSeg = W1;
                Tex.Texture = W1;

                if (Tex.Tile == 0xFF) {
                    Tex.ImgType = (F3DImageType)ImgType;
                    Tex.BitDepth = BitDepth;
                }
                break;
            }
            case G_SETTILE: {
                F3DTexture &Tex = GetCurrTex(Textures);
                u32 Tile = C1(24, 3);

                Tex.Tile = Tile;
                if (Tile != G_TX_LOADTILE) {
                    Tex.ImgType = (F3DImageType)C0(21, 3);
                    Tex.BitDepth = GetBitDepthFromSize(C0(19, 2));
                }
                break;
            }
            case G_LOADBLOCK: {
                F3DTexture &Tex = GetCurrTex(Textures);
                u32 Texels = C1(12, 12);
                u32 Bytes = ((Texels + 1) * CurrentLoadBitDepth) / 8;
                if (Bytes > Tex.Length) Tex.Length = Bytes;
                break;
            }
            case G_SETTILESIZE: {
                F3DTexture &Tex = GetCurrTex(Textures);
                u32 Tile = C1(24, 3);

                if (Tile != G_TX_LOADTILE) {
                    u16 W = (u16)(((C1(12, 12) - C0(12, 12)) >> 2) + 1);
                    u16 H = (u16)(((C1(0, 12) - C0(0, 12)) >> 2) + 1);

                    if (W > 0 && W <= 1024 && H > 0 && H <= 1024) {
                        Tex.Width = W;
                        Tex.Height = H;
                    }
                }
                break;
            }
            case G_LOADTLUT: {
                F3DTexture &Tex = GetCurrTex(Textures);
                Tex.Palette = Tex.Texture;
                Tex.PaletteSeg = Tex.TextureSeg;
                break;
            }
        }
        return;
    }
    
    auto GetPlaceHolderName = [LvlName, Area, IsActor, Act](void) {
        return IsActor ? Act->Name : std::format("{}_{}", LvlName, Area);
    };

    switch (Cmd) {
        case G_SETTIMG:
            if (ValidateMemAddr(W1)) {
                fprintf(ModelDump, "    gsDPSetTextureImage(%u, %u, 1, %s_texture_0x%x),\n", C0(21,3), C0(19,2), GetPlaceHolderName().c_str(), W1);
            } else {
                fprintf(ModelDump, "    // gsDPSetTextureImage(%u, %u, 1, %s_texture_0x%x),\n", C0(21,3), C0(19,2), GetPlaceHolderName().c_str(), W1);
            }
            break;
        case G_SETTILE:
            fprintf(ModelDump, "    gsDPSetTile(%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u),\n", 
                    C0(21,3), C0(19,2), C0(9,9), C0(0,9), C1(24,3), C1(20,4), C1(18,2), C1(14,4), C1(10,4), C1(8,2), C1(4,4), C1(0,4));
            break;
        case G_SETTILESIZE:
            fprintf(ModelDump, "    gsDPSetTileSize(%u, %u, %u, %u, %u),\n", 
                    C1(24,3), C0(12,12), C0(0,12), C1(12,12), C1(0,12));
            break;
        case G_LOADBLOCK:
            fprintf(ModelDump, "    gsDPLoadBlock(%u, %u, %u, %u, %u),\n", 
                    C1(24,3), C0(12,12), C0(0,12), C1(12,12), C1(0,12));
            break;
        case G_LOADTLUT:
            fprintf(ModelDump, "    //gsDPLoadTLUTCmd(%u, %u),\n", C1(24,3), C1(14,10));
            break;
        case G_SETENVCOLOR:
            fprintf(ModelDump, "    gsDPSetEnvColor(%u, %u, %u, %u),\n", 
                    C1(24, 8), C1(16, 8), C1(8, 8), C1(0, 8));
            break;
        case G_SETPRIMCOLOR:
            fprintf(ModelDump, "    gsDPSetPrimColor(0, 0, %u, %u, %u, %u),\n", 
                    C1(24, 8), C1(16, 8), C1(8, 8), C1(0, 8));
            break;
        case G_SETFOGCOLOR:
            fprintf(ModelDump, "    gsDPSetFogColor(%u, %u, %u, %u),\n", 
                    C1(24, 8), C1(16, 8), C1(8, 8), C1(0, 8));
            break;
        case G_SETCOMBINE: {
            fprintf(ModelDump, "    gsDPSetCombineLERP(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s),\n",
                F3D_CC(CC_PART_A, C0(20, 4)), F3D_CC(CC_PART_B, C1(28, 4)), F3D_CC(CC_PART_C, C0(15, 5)), F3D_CC(CC_PART_D, C1(15, 3)), 
                F3D_AC(CC_PART_A, C0(12, 3)), F3D_AC(CC_PART_B, C1(12, 3)), F3D_AC(CC_PART_C, C0(9, 3)),  F3D_AC(CC_PART_D, C1(9, 3)),
                F3D_CC(CC_PART_A, C0(5, 4)),  F3D_CC(CC_PART_B, C1(21, 4)), F3D_CC(CC_PART_C, C0(0, 5)),  F3D_CC(CC_PART_D, C1(6, 3)), 
                F3D_AC(CC_PART_A, C1(21, 3)), F3D_AC(CC_PART_B, C1(3, 3)),  F3D_AC(CC_PART_C, C1(18, 3)), F3D_AC(CC_PART_D, C1(0, 3)));
            break;
        }
    }
}

void ParseDisplayListRecursive(N64Rom &Rom, u32 DisplayList, std::vector<F3DVertex> &Vertices, std::vector<F3DLight> &Ambients, std::vector<F3DLight> &Lights, std::vector<F3DTexture> &Textures, std::vector<F3DDL> &DLs, std::vector<u32> &CallStack) {
    if (std::find(CallStack.begin(), CallStack.end(), DisplayList) != CallStack.end()) {
        return;
    }
    
    CallStack.push_back(DisplayList);

    bool isNewDL = true;
    for (const auto &DL : DLs) {
        if (DL.DisplayListSeg == DisplayList) {
            isNewDL = false;
            break;
        }
    }
    if (isNewDL) DLs.push_back({ DisplayList, DisplayList });

    u32 Entry = DisplayList;

    while (true) {
        u32 W0 = Rom.ReadBytes<u32>(Entry);
        u32 W1 = Rom.ReadBytes<u32>(Entry + 4);
        u8 Cmd = W0 >> 24;
        
        ParseRDPCommands(Textures, W0, W1, Cmd, false, false, nullptr);
        
        if (Rom.Microcode == UCODE_F3D) {
            if (Cmd == (u8)G_ENDDL) break;
            
            switch (Cmd) {
                case G_VTX:
                    Vertices.push_back({ W1, W1, (C0(0, 16)) / 16 }); 
                    break;
                case G_MOVEMEM:
                    (C0(16, 8) == 0x88 ? Ambients : Lights).push_back({ W1, W1 }); 
                    break;
                case G_DL: {
                    bool Branch = (C0(16, 1) == 0x01);
                    if (ValidateMemAddr(W1)) {
                        ParseDisplayListRecursive(Rom, W1, Vertices, Ambients, Lights, Textures, DLs, CallStack);
                    } else if (VerbosePrinting) {
                        printf("DisplayList 0x%08x has an invalid address, Skipping\n", W1);
                    }
                    if (Branch) return;
                    break;
                }
                case (u8)G_TRI1: 
                    PushActiveTextures(Textures);
                    break;
            }
        } else if (Rom.Microcode == UCODE_F3DEX2) {
            if (Cmd == G_ENDDL_F3DEX2) break;
            
            switch (Cmd) {
                case G_VTX_F3DEX2:
                    Vertices.push_back({ W1, W1, C0(12, 8) }); 
                    break;
                case G_MOVEMEM_F3DEX2:
                    (((C0(8, 8) * 8) / 24 - 2) == 1 ? Ambients : Lights).push_back({ W1, W1 }); 
                    break;
                case G_DL_F3DEX2: {
                    bool Branch = (C0(16, 1) == 0x01);
                    if (ValidateMemAddr(W1)) {
                        ParseDisplayListRecursive(Rom, W1, Vertices, Ambients, Lights, Textures, DLs, CallStack);
                    } else if (VerbosePrinting) {
                       printf("DisplayList 0x%08x has an invalid address, Skipping\n", W1);
                    }
                    if (Branch) return;
                    break;
                }
                case G_TRI1_F3DEX2:
                case G_TRI2_F3DEX2:
                    PushActiveTextures(Textures);
                    break;
            }
        } else if (Rom.Microcode == UCODE_F3DEX) {
            if (Cmd == (u8)G_ENDDL) break;
            
            switch (Cmd) {
                case G_VTX:
                    Vertices.push_back({ W1, W1, C0(10, 6) }); 
                    break;
                case G_MOVEMEM:
                    (C0(16, 8) == 0x88 ? Ambients : Lights).push_back({ W1, W1 }); 
                    break;
                case G_DL: {
                    bool Branch = (C0(16, 1) == 0x01);
                    if (ValidateMemAddr(W1)) {
                        ParseDisplayListRecursive(Rom, W1, Vertices, Ambients, Lights, Textures, DLs, CallStack);
                    } else if (VerbosePrinting) {
                        printf("DisplayList 0x%08x has an invalid address, Skipping\n", W1);
                    }
                    if (Branch) return;
                    break;
                }
                case G_TRI2_F3DEX:
                case (u8)G_TRI1:
                    PushActiveTextures(Textures);
                    break;
            }
        }
        Entry += 8;
    }
}

const std::vector<std::pair<u32, std::string>> GeoMacrosF3D = {
    {8192, "G_CULL_BACK"}, {12288, "G_CULL_BOTH"}, {4096, "G_CULL_FRONT"},
    {65536, "G_FOG"}, {131072, "G_LIGHTING"}, {4, "G_SHADE"},
    {512, "G_SHADING_SMOOTH"}, {262144, "G_TEXTURE_GEN"},
    {524288, "G_TEXTURE_GEN_LINEAR"}, {1, "G_ZBUFFER"}
};

const std::vector<std::pair<u32, std::string>> GeoMacrosF3DEX2 = {
    {1024, "G_CULL_BACK"}, {1536, "G_CULL_BOTH"}, {512, "G_CULL_FRONT"},
    {65536, "G_FOG"}, {131072, "G_LIGHTING"}, {4, "G_SHADE"},
    {2097152, "G_SHADING_SMOOTH"}, {262144, "G_TEXTURE_GEN"},
    {524288, "G_TEXTURE_GEN_LINEAR"}, {1, "G_ZBUFFER"}
};

std::string ConvertGeoMode(N64Rom &Rom, u32 Flags) {
    std::string Result = "";
    const auto &Macros = (Rom.Microcode == UCODE_F3DEX2) ? GeoMacrosF3DEX2 : GeoMacrosF3D;

    for (const auto &Macro : Macros) {
        if ((Flags & Macro.first) == Macro.first) {
            Flags ^= Macro.first;
            Result += Macro.second + "|";
        }
    }
    
    if (!Result.empty()) Result.pop_back();
    else Result = "0";
    
    return Result;
}

void ExportModels(N64Rom &Rom, LevelScript &Script, const std::string &LvlName, u8 Area, const char *FilePath, bool IsActor, Actor *Act) {
    FILE *ModelDump = fopen(FilePath, "w");

    auto GetPlaceHolderName = [LvlName, Area, IsActor, Act](void) {
        return IsActor ? Act->Name : std::format("{}_{}", LvlName, Area);
    };

    std::vector<F3DVertex> Vertices;
    std::vector<F3DLight> Ambients;
    std::vector<F3DLight> Lights;
    std::vector<F3DTexture> Textures;
    std::vector<F3DDL> DLs;

    const std::string TextureFormatNames[] = { "rgba", "yuv", "ci", "ia", "i" };
    
    if (!IsActor) {
        for (u32 Data : Script.AreaDatas[Area].DisplayLists) {
            std::vector<u32> CallStack = {};
            ParseDisplayListRecursive(Rom, Data, Vertices, Ambients, Lights, Textures, DLs, CallStack);
        }
    } else {
        for (u32 ActorDL : Act->DisplayLists) {
            std::vector<u32> CallStack = {};
            ParseDisplayListRecursive(Rom, ActorDL, Vertices, Ambients, Lights, Textures, DLs, CallStack);
        }
    }

    std::set<u32> ExportedVertices, ExportedAmbients, ExportedLights, ExportedTextures;
    const std::string PlaceHolderNameStr = GetPlaceHolderName();
    const char *PlaceHolderName = PlaceHolderNameStr.c_str();

    for (const auto &V : Vertices) {
        if (!ValidateMemAddr(V.Vtx) || !ExportedVertices.insert(V.VtxSeg).second) continue;
        if (!IsActor) Script.AreaDatas[Area].Vertices.insert(V.VtxSeg);

        fprintf(ModelDump, "Vtx %s_vertex_0x%x[] = {\n", PlaceHolderName, V.VtxSeg);
        for (u32 I = 0; I < V.Size; I++) {
            u32 Addr = V.Vtx + I * 16;
            fprintf(ModelDump, "    {{{ %d, %d, %d }, 0, { %d, %d }, { %u, %u, %u, %u }}},\n",
                    Rom.ReadBytes<s16>(Addr + 0), Rom.ReadBytes<s16>(Addr + 2), Rom.ReadBytes<s16>(Addr + 4),
                    Rom.ReadBytes<s16>(Addr + 8), Rom.ReadBytes<s16>(Addr + 10),
                    Rom.ReadBytes<u8>(Addr + 12), Rom.ReadBytes<u8>(Addr + 13), Rom.ReadBytes<u8>(Addr + 14), Rom.ReadBytes<u8>(Addr + 15));
        }
        fprintf(ModelDump, "};\n\n");
    }

    for (const auto &A : Ambients) {
        if (!ValidateMemAddr(A.Light) || !ExportedAmbients.insert(A.LightSeg).second) continue;

        fprintf(ModelDump, "Ambient_t %s_light_0x%x[] = {\n", PlaceHolderName, A.LightSeg);
        u32 Addr = A.Light;
        fprintf(ModelDump, "    { %u, %u, %u}, 0, { %u, %u, %u}, 0\n};\n\n",
                Rom.ReadBytes<u8>(Addr + 0), Rom.ReadBytes<u8>(Addr + 1), Rom.ReadBytes<u8>(Addr + 2),
                Rom.ReadBytes<u8>(Addr + 4), Rom.ReadBytes<u8>(Addr + 5), Rom.ReadBytes<u8>(Addr + 6));
    }

    for (const auto &L : Lights) {
        if (!ValidateMemAddr(L.Light) || !ExportedLights.insert(L.LightSeg).second) continue;

        fprintf(ModelDump, "Light_t %s_light_0x%x[] = {\n", PlaceHolderName, L.LightSeg);
        u32 Addr = L.Light;
        fprintf(ModelDump, "    { %u, %u, %u}, 0, { %u, %u, %u}, 0, { %d, %d, %d}, 0\n};\n\n",
                Rom.ReadBytes<u8>(Addr + 0), Rom.ReadBytes<u8>(Addr + 1), Rom.ReadBytes<u8>(Addr + 2),
                Rom.ReadBytes<u8>(Addr + 4), Rom.ReadBytes<u8>(Addr + 5), Rom.ReadBytes<u8>(Addr + 6),
                Rom.ReadBytes<s8>(Addr + 8), Rom.ReadBytes<s8>(Addr + 9), Rom.ReadBytes<s8>(Addr + 10));
    }

    for (auto &T : Textures) {
        if (!ValidateMemAddr(T.Texture) || !ExportedTextures.insert(T.TextureSeg).second) continue;

        u32 PixelCount = T.Width * T.Height;
        if (T.Length == 0) T.Length = ((PixelCount + 1) * (u32)T.BitDepth) / 8;

        std::vector<u8> SrcData(T.Length * 2);
        for (u32 j = 0; j < T.Length; j++) SrcData[j] = Rom.ReadBytes<u8>(T.Texture + j);

        std::vector<u8> RGBA(PixelCount * 4);
        switch (T.ImgType) {
            case F3D_IMG_RGBA:
            case F3D_IMG_YUV:
                if (T.BitDepth == 16) BinImg::DecodeRGBA16(SrcData.data(), RGBA.data(), PixelCount);
                else if (T.BitDepth == 32) BinImg::DecodeRGBA32(SrcData.data(), RGBA.data(), PixelCount);
                break;
            case F3D_IMG_CI:
                if (T.BitDepth == 4) BinImg::DecodeCI4(SrcData.data(), RGBA.data(), PixelCount, Rom, T.Palette);
                else if (T.BitDepth == 8) BinImg::DecodeCI8(SrcData.data(), RGBA.data(), PixelCount, Rom, T.Palette);
                break;
            case F3D_IMG_I:
                if (T.BitDepth == 4) BinImg::DecodeI4(SrcData.data(), RGBA.data(), PixelCount);
                else if (T.BitDepth == 8) BinImg::DecodeI8(SrcData.data(), RGBA.data(), PixelCount);
                break;
            case F3D_IMG_IA:
                if (T.BitDepth == 4) BinImg::DecodeIA4(SrcData.data(), RGBA.data(), PixelCount);
                else if (T.BitDepth == 8) BinImg::DecodeIA8(SrcData.data(), RGBA.data(), PixelCount);
                else if (T.BitDepth == 16) BinImg::DecodeIA16(SrcData.data(), RGBA.data(), PixelCount);
                break;
            default:
                T.ImgType = F3D_IMG_RGBA;
                break;
        }

        char ImgTypeName[64], FileName[256];
        sprintf(ImgTypeName, "%s%u", TextureFormatNames[T.ImgType].c_str(), T.BitDepth);
        
        const char* SubDir = IsActor ? "actors" : "levels";
        const char* TargetName = IsActor ? Act->Name.c_str() : LvlName.c_str();

        sprintf(FileName, "output/%s/%s/%s_texture_0x%x.%s.png", SubDir, TargetName, PlaceHolderName, T.TextureSeg, ImgTypeName);
        stbi_write_png(FileName, T.Width, T.Height, 4, RGBA.data(), T.Width * 4);

        fprintf(ModelDump, "u8 %s_texture_0x%x[] = {\n", PlaceHolderName, T.TextureSeg);
        fprintf(ModelDump, "    #include \"%s/%s/%s_texture_0x%x.%s.inc.c\"\n};\n", SubDir, TargetName, PlaceHolderName, T.TextureSeg, ImgTypeName);

        if (T.ImgType == F3D_IMG_CI) {
            fprintf(ModelDump, "u8 %s_texture_0x%x[] = {\n    0x00\n};\n", PlaceHolderName, T.PaletteSeg);
        }
    }

    for (const auto &DL : DLs) {
        u32 Entry = DL.Entry;
        fprintf(ModelDump, "Gfx %s_displaylist_0x%x[] = {\n", PlaceHolderName, DL.DisplayListSeg);

        while (true) {
            u32 W0 = Rom.ReadBytes<u32>(Entry);
            u32 W1 = Rom.ReadBytes<u32>(Entry + 4);
            u8 Cmd = W0 >> 24;
            bool ShouldEnd = false;
            
            ParseRDPCommands(Textures, W0, W1, Cmd, true, IsActor, Act, ModelDump, LvlName, Area);

            if (Rom.Microcode == UCODE_F3D) {
                if (Cmd == (u8)G_ENDDL) { fprintf(ModelDump, "    gsSPEndDisplayList(),\n};\n\n"); break; }

                switch(Cmd) {
                    case G_VTX:
                        if (ValidateMemAddr(W1)) {
                            fprintf(ModelDump, "    gsSPVertex(%s_vertex_0x%x, %u, %u),\n", PlaceHolderName, W1, C0(0,16)/16, C0(16,4));
                        } else {
                            fprintf(ModelDump, "    // gsSPVertex(%s_vertex_0x%x, %u, %u),\n", PlaceHolderName, W1, C0(0,16)/16, C0(16,4));
                        }
                        break;
                    case (u8)G_TRI1: {
                        u32 NextW0 = Rom.ReadBytes<u32>(Entry + 8);
                        u32 NextW1 = Rom.ReadBytes<u32>(Entry + 12);
                        if ((NextW0 >> 24) == 0xbf) {
                            u32 V00 = C1(16, 8) / 10, V01 = C1(8, 8) / 10, V02 = C1(0, 8) / 10;
                            u32 V10 = (NextW1 >> 16 & 0xFF) / 10, V11 = (NextW1 >> 8 & 0xFF) / 10, V12 = (NextW1 & 0xFF) / 10;
                            fprintf(ModelDump, "    gsSP2Triangles(%u, %u, %u, 0, %u, %u, %u, 0),\n", V00, V01, V02, V10, V11, V12);
                            Entry += 8;
                        } else {
                            fprintf(ModelDump, "    gsSP1Triangle(%u, %u, %u, 0),\n", C1(16, 8) / 10, C1(8, 8) / 10, C1(0, 8) / 10);
                        }
                        break;
                    }
                    case (u8)G_CLEARGEOMETRYMODE:
                        fprintf(ModelDump,"    gsSPClearGeometryMode(%s),\n", ConvertGeoMode(Rom, W1).c_str());
                        break;
                    case (u8)G_SETGEOMETRYMODE:
                        fprintf(ModelDump,"    gsSPSetGeometryMode(%s),\n", ConvertGeoMode(Rom, W1).c_str());
                        break;
                    case G_MOVEMEM:
                        fprintf(ModelDump,"    gsSPLight(&%s_light_0x%x.col, %u),\n", PlaceHolderName, W1, (C0(16,8) == 0x88 ? 2 : 1));
                        break;
                    case G_DL: {
                        bool Branch = (C0(16, 1) == 0x01);
                        fprintf(ModelDump,"    %s(%s_displaylist_0x%x),\n", Branch ? "gsSPBranchList" : "gsSPDisplayList", PlaceHolderName, W1);
                        if (Branch) ShouldEnd = true;
                        break;
                    }
                    case (u8)G_TEXTURE:
                        fprintf(ModelDump,"    gsSPTexture(%u, %u, %u, %u, %u),\n", C1(16,16), C1(0,16), C0(11,3), C0(8,3), C0(0,8));
                        break;
                    case (u8)G_SETOTHERMODE_L:
                    case (u8)G_SETOTHERMODE_H:
                        fprintf(ModelDump,"    gsSPSetOtherMode(%u, %u, %u, %u),\n", Cmd, C0(8, 8), C0(0, 8), W1);
                        break;
                }
            } else if (Rom.Microcode == UCODE_F3DEX2) {
                if (Cmd == G_ENDDL_F3DEX2) { fprintf(ModelDump, "    gsSPEndDisplayList(),\n};\n\n"); break; }

                switch(Cmd) {
                    case G_VTX_F3DEX2:
                        if (ValidateMemAddr(W1)) {
                            fprintf(ModelDump, "    gsSPVertex(%s_vertex_0x%x, %u, %u),\n", PlaceHolderName, W1, C0(12, 8), (C0(1, 7) - C0(12, 8)));
                        } else {
                            fprintf(ModelDump, "    // gsSPVertex(%s_vertex_0x%x, %u, %u),\n", PlaceHolderName, W1, C0(12, 8), (C0(1, 7) - C0(12, 8)));
                        }
                        break;
                    case G_TRI1_F3DEX2:
                        fprintf(ModelDump, "    gsSP1Triangle(%u, %u, %u, 0),\n", C0(16, 8) / 2, C0(8, 8) / 2, C0(0, 8) / 2);
                        break;
                    case G_TRI2_F3DEX2:
                        fprintf(ModelDump, "    gsSP2Triangles(%u, %u, %u, 0, %u, %u, %u, 0),\n", 
                                C0(16, 8)/2, C0(8, 8)/2, C0(0, 8)/2, C1(16, 8)/2, C1(8, 8)/2, C1(0, 8)/2);
                        break;
                    case G_DL_F3DEX2: {
                        bool Branch = (C0(16, 1) == 0x01);
                        fprintf(ModelDump,"    %s(%s_displaylist_0x%x),\n", Branch ? "gsSPBranchList" : "gsSPDisplayList", PlaceHolderName, W1);
                        if (Branch) ShouldEnd = true;
                        break;
                    }
                    case G_TEXTURE_F3DEX2:
                        fprintf(ModelDump,"    gsSPTexture(%u, %u, %u, %u, %u),\n", C1(16, 16), C1(0, 16), C0(11, 3), C0(8, 3), C0(1, 7));
                        break;
                    case G_GEOMETRYMODE_F3DEX2:
                        fprintf(ModelDump,"    gsSPGeometryMode(%s, %s),\n", ConvertGeoMode(Rom, ~C0(0, 24)).c_str(), ConvertGeoMode(Rom, W1).c_str());
                        break;
                    case G_MOVEMEM_F3DEX2:
                        fprintf(ModelDump,"    gsSPLight(&%s_light_0x%x.col, %u),\n", PlaceHolderName, W1, (((C0(8, 8) * 8) / 24 - 2) == 1 ? 2 : 1));
                        break;
                    case (u8)G_SETOTHERMODE_L_F3DEX2:
                    case (u8)G_SETOTHERMODE_H_F3DEX2:
                        fprintf(ModelDump,"    gsSPSetOtherMode(%u, %u, %u, %u),\n", Cmd, 31 - C0(8, 8) - C0(0, 8), C0(0, 8) + 1, W1);
                        break;
                }
            } else if (Rom.Microcode == UCODE_F3DEX) {
                if (Cmd == (u8)G_ENDDL) { fprintf(ModelDump, "    gsSPEndDisplayList(),\n};\n\n"); break; }

                switch(Cmd) {
                    case G_VTX:
                        if (ValidateMemAddr(W1)) {
                            fprintf(ModelDump, "    gsSPVertex(%s_vertex_0x%x, %u, %u),\n", PlaceHolderName, W1, C0(10,6), C0(16, 8) / 2);
                        } else {
                            fprintf(ModelDump, "    // gsSPVertex(%s_vertex_0x%x, %u, %u),\n", PlaceHolderName, W1, C0(10,6), C0(16, 8) / 2);
                        }
                        break;
                    case (u8)G_TRI1:
                        fprintf(ModelDump, "    gsSP1Triangle(%u, %u, %u, 0),\n", C1(16, 8) / 2, C1(8, 8) / 2, C1(0, 8) / 2);
                        break;
                    case G_TRI2_F3DEX:
                        fprintf(ModelDump, "    gsSP2Triangles(%u, %u, %u, 0, %u, %u, %u, 0),\n", 
                                C0(16, 8)/2, C0(8, 8)/2, C0(0, 8)/2, C1(16, 8)/2, C1(8, 8)/2, C1(0, 8)/2);
                        break;
                    case (u8)G_CLEARGEOMETRYMODE:
                        fprintf(ModelDump,"    gsSPClearGeometryMode(%s),\n", ConvertGeoMode(Rom, W1).c_str());
                        break;
                    case (u8)G_SETGEOMETRYMODE:
                        fprintf(ModelDump,"    gsSPSetGeometryMode(%s),\n", ConvertGeoMode(Rom, W1).c_str());
                        break;
                    case G_MOVEMEM:
                        fprintf(ModelDump,"    gsSPLight(&%s_light_0x%x.col, %u),\n", PlaceHolderName, W1, (C0(16,8) == 0x88 ? 2 : 1));
                        break;
                    case G_DL: {
                        bool Branch = (C0(16, 1) == 0x01);
                        fprintf(ModelDump,"    %s(%s_displaylist_0x%x),\n", Branch ? "gsSPBranchList" : "gsSPDisplayList", PlaceHolderName, W1);
                        if (Branch) ShouldEnd = true;
                        break;
                    }
                    case (u8)G_TEXTURE:
                        fprintf(ModelDump,"    gsSPTexture(%u, %u, %u, %u, %u),\n", C1(16,16), C1(0,16), C0(11,3), C0(8,3), C0(0,8));
                        break;
                    case (u8)G_SETOTHERMODE_L:
                    case (u8)G_SETOTHERMODE_H:
                        fprintf(ModelDump,"    gsSPSetOtherMode(%u, %u, %u, %u),\n", Cmd, C0(8, 8), C0(0, 8), W1);
                        break;
                }
            }

            if (ShouldEnd) {
                fprintf(ModelDump, "};\n\n");
                break;
            }

            Entry += 8;
        }
    }
    
    fclose(ModelDump);
}