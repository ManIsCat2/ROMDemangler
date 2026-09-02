#pragma once

#include "ultratypes.h"
#include "Global.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <bit>

enum SM64GameTypeID {
    GT_UNKNOWN,
    GT_ROM_MANAGER,
    GT_EDITOR,
    GT_LEVEL_IMPORTER,
    GT_BBP,
    GT_DECOMP,
    GT_HACKER
};

struct SM64GameType {
    u8 ID = GT_UNKNOWN;

    void SetID(u8 NewID) {
        ID = NewID;
    }
    bool IsOldBinary(void) {
        return (ID == GT_ROM_MANAGER || ID == GT_EDITOR || ID == GT_LEVEL_IMPORTER);
    }
    bool IsNewBinary(void) {
        return (ID == GT_BBP);
    }
    bool IsBinary(void) {
        return IsOldBinary() || IsNewBinary();
    }
    bool IsDecomp(void) {
        return (ID == GT_DECOMP || ID == GT_HACKER);
    }
    bool IsHacker(void) {
        return (ID == GT_HACKER);
    }
    u8 GetID(void) {
        return ID;
    }
};

enum N64Microcode {
    UCODE_UNKNOWN = 0,
    UCODE_F3D,
    UCODE_F3DEX,
    UCODE_F3DEX2,
    UCODE_F3DZEX,
};

extern SM64GameType GameType;
extern bool ExportSegment0;

extern std::vector<u8> SegmentData[MAX_SEGMENT];
extern u32 SegmentOffsets[MAX_SEGMENT][2];

class N64Rom {
public:
    FILE *File;
    size_t Size = 0;
    u8 *Data = nullptr;
    u8 *RAM = nullptr;
    std::string ROMInternalName;
    enum N64Microcode Microcode = UCODE_UNKNOWN;

    void OpenFile(const char *Path, const char *RAMPath);

    template <typename T>
    T ReadBytesPhysical(s32 Offset) {
        T Buf{};
        size_t RealSize = sizeof(T);
        if (Offset + RealSize > Size) return Buf;
        memcpy(&Buf, Data + Offset, RealSize);
        if (RealSize > 1) {
            Buf = SwapEndian(Buf);
        }
        return Buf;
    }
    template <typename T>
    T ReadBytes(u32 SegAddr, bool Seg0AsRAM=true) {
        T Buf{};    
        u8 Bank = SegAddr >> 24;
        u32 Offset = SegAddr & 0xFFFFFF;

        if (Bank > (MAX_SEGMENT - 1)) {
            return 0;
        }

        if (Bank != 0 && !SegmentData[Bank].empty()) {
            const auto &Seg = SegmentData[Bank];
            size_t RealSize = sizeof(T);
            if (Offset + RealSize > Seg.size()) return Buf;

            memcpy(&Buf, Seg.data() + Offset, RealSize);
            if (RealSize > 1) {
                Buf = SwapEndian(Buf);
            }
            return Buf;
        }

        if (Bank == 0) {
            if (ExportSegment0) {
                if (Seg0AsRAM) {
                    T Buf{};
                    size_t RealSize = sizeof(T);
                    memcpy(&Buf, RAM + Offset, RealSize);
                    if (RealSize > 1) {
                        Buf = SwapEndian(Buf);
                    }
                    return Buf;
                }
            } else {
                if (GameType.IsOldBinary()) {
                    if (Offset > 0x400000 and Offset < 0x420000) {
                        Offset = 0x1200000 + (SegAddr & 0xFFFFF);
                    } else {
                        if (Offset > 0x5F0000 and Offset < 0x620000) {
                            Offset += 0x1E0000;
                        }
                    }
                }
            }
        }

        return ReadBytesPhysical<T>(Offset);
    }

    template <typename T>
    T *ReadBytesPtr(u32 Offset, T *Buf, u32 Len) {
        size_t RealSize = sizeof(T) * Len;
        if (Offset + RealSize > Size) return Buf;
        memcpy(Buf, Data + Offset, RealSize);

        if (sizeof(T) > 1) {
            for (u32 i = 0; i < Len; i++) {
                Buf[i] = SwapEndian(Buf[i]);
            }
        }
        return Buf;
    }

    template <typename T>
    static inline T SwapEndian(T Val) {
        if constexpr(sizeof(T) == 1) {
            return Val;
        } else if constexpr(std::is_integral_v<T>) {
            return std::byteswap(Val);
        } else if constexpr(std::is_floating_point_v<T>) {
            if constexpr(sizeof(T) == 4) {
                auto Bits = std::bit_cast<u32>(Val);
                return std::bit_cast<T>(std::byteswap(Bits));
            } 
            else if constexpr(sizeof(T) == 8) {
                auto Bits = std::bit_cast<u64>(Val);
                return std::bit_cast<T>(std::byteswap(Bits));
            }
        }
        return Val;
    }

    void CloseFile(void);
};