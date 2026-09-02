#include <cstring>
#include "BinImg.h"

#define SCALE_5_8(VAL) (((VAL) << 3) | ((VAL) >> 2))

namespace BinImg {
    void DecodeRGBA16(const u8 *Src, u8 *Dst, u32 Pixels) {
        for (u32 I = 0; I < Pixels; I++) {
            u16 C = ((u16)Src[I * 2] << 8) | (u16)Src[I * 2 + 1];
            u8 R = (C >> 11) & 0x1F;
            u8 G = (C >> 6)  & 0x1F;
            u8 B = (C >> 1)  & 0x1F;

            Dst[I * 4 + 0] = SCALE_5_8(R);
            Dst[I * 4 + 1] = SCALE_5_8(G);
            Dst[I * 4 + 2] = SCALE_5_8(B);
            Dst[I * 4 + 3] = (C & 1) ? 255 : 0;
        }
    }

    void DecodeRGBA32(const u8 *Src, u8 *Dst, u32 Pixels) {
        memcpy(Dst, Src, Pixels * 4);
    }

    void DecodeCI4(const u8 *Src, u8 *Dst, u32 Pixels, N64Rom &Rom, u32 PalAddr) {
        u16 Palette[16];
        for (int i = 0; i < 16; i++) Palette[i] = Rom.ReadBytes<u16>(PalAddr + i * 2);
        for (u32 i = 0; i < Pixels; i++) {
            u8 Idx = (i & 1) ? Src[i/2] & 0xF : Src[i/2] >> 4;
            Dst[i*4+0] = ((Palette[Idx] >> 11) & 0x1F) << 3;
            Dst[i*4+1] = ((Palette[Idx] >> 6) & 0x1F) << 3;
            Dst[i*4+2] = ((Palette[Idx] >> 1) & 0x1F) << 3;
            Dst[i*4+3] = (Palette[Idx] & 1) ? 255 : 0;
        }
    }

    void DecodeCI8(const u8 *Src, u8 *Dst, u32 Pixels, N64Rom &Rom, u32 PalAddr) {
        u16 Palette[256];
        for (int i = 0; i < 256; i++) Palette[i] = Rom.ReadBytes<u16>(PalAddr + i * 2);
        for (u32 i = 0; i < Pixels; i++) {
            u8 Idx = Src[i];
            Dst[i*4+0] = ((Palette[Idx] >> 11) & 0x1F) << 3;
            Dst[i*4+1] = ((Palette[Idx] >> 6) & 0x1F) << 3;
            Dst[i*4+2] = ((Palette[Idx] >> 1) & 0x1F) << 3;
            Dst[i*4+3] = (Palette[Idx] & 1) ? 255 : 0;
        }
    }


    void DecodeI4(const u8 *Src, u8 *Dst, u32 Pixels) {
        for (u32 I = 0; I < Pixels; I++) {
            u8 V = Src[I / 2];
            u8 N = (I & 1) ? (V & 0xF) : (V >> 4);
            u8 Intensity = (N << 4) | N;
            Dst[I * 4 + 0] = Dst[I * 4 + 1] = Dst[I * 4 + 2] = Intensity;
            Dst[I * 4 + 3] = 255;
        }
    }

    void DecodeI8(const u8 *Src, u8 *Dst, u32 Pixels) {
        for (u32 I = 0; I < Pixels; I++) {
            u8 Intensity = Src[I];
            Dst[I * 4 + 0] = Dst[I * 4 + 1] = Dst[I * 4 + 2] = Intensity;
            Dst[I * 4 + 3] = 255;
        }
    }

    void DecodeIA4(const u8 *Src, u8 *Dst, u32 Pixels) {
        for (u32 I = 0; I < Pixels; I++) {
            u8 V = Src[I / 2];
            u8 N = (I & 1) ? (V & 0xF) : (V >> 4);
            u8 Intensity = (N >> 1) * 0x24;
            u8 Alpha = (N & 1) ? 255 : 0;
            Dst[I * 4 + 0] = Dst[I * 4 + 1] = Dst[I * 4 + 2] = Intensity;
            Dst[I * 4 + 3] = Alpha;
        }
    }

    void DecodeIA8(const u8 *Src, u8 *Dst, u32 Pixels) {
        for (u32 I = 0; I < Pixels; I++) {
            u8 V = Src[I];
            u8 Intensity = V >> 4;
            u8 Alpha = V & 0xF;
            Intensity = (Intensity << 4) | Intensity;
            Alpha = (Alpha << 4) | Alpha;
            Dst[I * 4 + 0] = Intensity;
            Dst[I * 4 + 1] = Intensity;
            Dst[I * 4 + 2] = Intensity;
            Dst[I * 4 + 3] = Alpha;
        }
    }

    void DecodeIA16(const u8 *Src, u8 *Dst, u32 Pixels) {
        for (u32 I = 0; I < Pixels; I++) {
            u8 Intensity = Src[I * 2];
            u8 Alpha = Src[I * 2 + 1];
            Dst[I * 4 + 0] = Dst[I * 4 + 1] = Dst[I * 4 + 2] = Intensity;
            Dst[I * 4 + 3] = Alpha;
        }
    }
} // namespace BinImg