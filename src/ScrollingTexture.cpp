#include "ScrollingTexture.h"

std::vector<ScrollTexture> ScrollingTextures = {};

u8 GetPosByte(f32 Val) {
    u32 Bits = std::bit_cast<u32>(Val);
    return (Bits >> 16) & 0xFF;
}

u16 GetScrollAxis(u16 Dir) {
    u16 V = Dir & 0xF000;
    switch (V) {
        case 0xA000: return 4; // x
        case 0x8000: return 5; // y
        case 0x4000: return 0; // xpos
        case 0x2000: return 1; // ypos
        case 0x0000: return 2; // zpos
        default: return 4;
    }
}

u16 GetScrollType(u16 Dir) {
    u16 V = Dir & 0x0F00;
    switch (V) {
        case 0x000: return 0; // normal
        case 0x100: return 1; // sine
        case 0x200: return 2; // jump
        default: return 0;
    }
}

bool UsesNewEditorScroll(N64Rom &Rom) {
    u32 Val = Rom.ReadBytesPhysical<u32>(0x1202400);
    return Val != 0x27bdffe8;
}

ScrollTexture ConvertRMTexScrolls(LevelScript &Script, u32 Bparam, u16 NumVtx, u16 Dir, s16 Speed) {
    ScrollTexture S;
    S.Addr = Bparam;
    S.NumVtx = NumVtx;
    S.Speed = Speed;
    S.Axis = GetScrollAxis(Dir);
    S.Type = GetScrollType(Dir);
    S.Cycle = Dir & 0xFF;
    S.Area = Script.CurrArea;
    S.Id = (Script.LevelID << 16) | (Script.ScrollTargets.size() & 0xFF);

    return S;
}

// this is the second most depressing function i have ever wrote
ScrollTexture ConvertEditorTexScrolls(LevelScript &Script, u32 Bparam, s16 PosX, s16 PosY, s16 PosZ, std::string &BhvName, N64Rom& Rom) {
    ScrollTexture S;
    
    u32 Addr = 0x0E000000 + ((GetPosByte(PosX) - 2) << 16) + (Bparam >> 16);
    u32 Dir = Addr & 0xF;

    if (BhvName != "editor_Scroll_Texture2" && UsesNewEditorScroll(Rom)) {
        S.NumVtx = Bparam & 0xFFFF;
    } else {
        if (BhvName == "editor_Scroll_Texture2") {
            BhvName = "editor_Scroll_Texture";
        }
        if (PosY != 0) {
            S.NumVtx = GetPosByte(PosY) * 3;
        } else {
            S.NumVtx = 0;
        }
    }

    S.Addr = Addr & 0xFFFFFFF0;
    S.Speed = GetPosByte(PosZ);
    S.Axis = (Dir == 0x8) ? 4 : 5;
    S.Type = 0;
    S.Cycle = 0;
    S.Area = Script.CurrArea;
    S.Id = (Script.LevelID << 16) | (Script.ScrollTargets.size() & 0xFFFF);

    return S;
}

void ResolveScrollTargets(LevelScript &Script) {
    for (const auto &P : Script.ScrollTargets) {
        std::set<u32> &Vertices = Script.AreaDatas[P.Area].Vertices;
        u32 BaseAddr = P.Addr;

        for (int i = 0; i < 15; i++) {
            if (Vertices.count(BaseAddr)) {
                break;
            }

            BaseAddr -= 0x10;
        }

        int i = 0;
        while (i < P.NumVtx) {
            ScrollTexture S = {};

            u32 VtxAddr = P.Addr + i * 0x10;
            u32 VbStart = BaseAddr + ((VtxAddr - BaseAddr) / 0xF0) * 0xF0;
            u32 VtxOffset = (VtxAddr - VbStart) / 0x10;
            u32 VtxCount = MIN(15 - VtxOffset, P.NumVtx - i);

            S.Speed = P.Speed;
            S.Axis = P.Axis;
            S.Type = P.Type;
            S.Cycle = P.Cycle;
            S.Area = P.Area;
            S.Id = P.Id;
            S.Offset = VtxOffset;
            S.NumVtx = VtxCount;

            S.Addr = VtxAddr;
            S.Name = std::format("{}_{}_vertex_0x{:x}", Script.Name, S.Area, VbStart);

            ScrollingTextures.push_back(S);
            i += VtxCount;
        }
    }
}