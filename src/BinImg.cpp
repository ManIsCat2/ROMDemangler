#include <cstring>
#include "BinImg.h"

#define SCALE_5_8(VAL) (((VAL) * 0xFF) / 0x1F)
#define SCALE_8_5(VAL) ((((VAL) + 4) * 0x1F) / 0xFF)
#define SCALE_4_8(VAL) ((VAL) * 0x11)
#define SCALE_8_4(VAL) ((VAL) / 0x11)
#define SCALE_3_8(VAL) (((VAL) * 0xFF) / 0x07)
#define SCALE_8_3(VAL) (((VAL) * 0x07) / 0xFF)

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

        for (s32 I = 0; I < 16; I++) {
            Palette[I] = Rom.ReadBytes<u16>(PalAddr + I * 2);
        }

        for (u32 I = 0; I < Pixels; I++) {
            u8 Idx = (I & 1) ? (Src[I / 2] & 0x0F) : (Src[I / 2] >> 4);

            u16 C = Palette[Idx];

            u8 R = (C >> 11) & 0x1F;
            u8 G = (C >> 6)  & 0x1F;
            u8 B = (C >> 1)  & 0x1F;

            Dst[I * 4 + 0] = SCALE_5_8(R);
            Dst[I * 4 + 1] = SCALE_5_8(G);
            Dst[I * 4 + 2] = SCALE_5_8(B);
            Dst[I * 4 + 3] = (C & 1) ? 255 : 0;
        }
    }

    void DecodeCI8(const u8 *Src, u8 *Dst, u32 Pixels, N64Rom &Rom, u32 PalAddr) {
        u16 Palette[256];

        for (s32 I = 0; I < 256; I++) {
            Palette[I] = Rom.ReadBytes<u16>(PalAddr + I * 2);
        }

        for (u32 I = 0; I < Pixels; I++) {
            u8 Idx = Src[I];

            u16 C = Palette[Idx];

            u8 R = (C >> 11) & 0x1F;
            u8 G = (C >> 6)  & 0x1F;
            u8 B = (C >> 1)  & 0x1F;

            Dst[I * 4 + 0] = SCALE_5_8(R);
            Dst[I * 4 + 1] = SCALE_5_8(G);
            Dst[I * 4 + 2] = SCALE_5_8(B);
            Dst[I * 4 + 3] = (C & 1) ? 255 : 0;
        }
    }


    void DecodeI4(const u8 *Src, u8 *Dst, u32 Pixels) {
        for (u32 I = 0; I < Pixels; I++) {
            u8 V = Src[I / 2];

            u8 Intensity = (I & 1) ? (V & 0x0F) : (V >> 4);

            Intensity = SCALE_4_8(Intensity);

            Dst[I * 4 + 0] = Intensity;
            Dst[I * 4 + 1] = Intensity;
            Dst[I * 4 + 2] = Intensity;
            Dst[I * 4 + 3] = 255;
        }
    }


    void DecodeI8(const u8 *Src, u8 *Dst, u32 Pixels) {
        for (u32 I = 0; I < Pixels; I++) {
            u8 Intensity = Src[I];

            Dst[I * 4 + 0] = Intensity;
            Dst[I * 4 + 1] = Intensity;
            Dst[I * 4 + 2] = Intensity;
            Dst[I * 4 + 3] = 255;
        }
    }


    void DecodeIA4(const u8 *Src, u8 *Dst, u32 Pixels) {
        for (u32 I = 0; I < Pixels; I++) {
            u8 V = Src[I / 2];

            u8 N = (I & 1) ? (V & 0x0F) : (V >> 4);

            u8 Intensity = N >> 1;
            u8 Alpha = N & 1;

            Intensity = SCALE_3_8(Intensity);

            Dst[I * 4 + 0] = Intensity;
            Dst[I * 4 + 1] = Intensity;
            Dst[I * 4 + 2] = Intensity;
            Dst[I * 4 + 3] = Alpha ? 255 : 0;
        }
    }

    void DecodeIA8(const u8 *Src, u8 *Dst, u32 Pixels) {
        for (u32 I = 0; I < Pixels; I++) {
            u8 V = Src[I];

            u8 Intensity = V >> 4;
            u8 Alpha = V & 0x0F;

            Intensity = SCALE_4_8(Intensity);
            Alpha = SCALE_4_8(Alpha);

            Dst[I * 4 + 0] = Intensity;
            Dst[I * 4 + 1] = Intensity;
            Dst[I * 4 + 2] = Intensity;
            Dst[I * 4 + 3] = Alpha;
        }
    }

    void DecodeIA16(const u8 *Src, u8 *Dst, u32 Pixels) {
        for (u32 I = 0; I < Pixels; I++) {
            u8 Intensity = Src[I * 2 + 0];
            u8 Alpha = Src[I * 2 + 1];

            Dst[I * 4 + 0] = Intensity;
            Dst[I * 4 + 1] = Intensity;
            Dst[I * 4 + 2] = Intensity;
            Dst[I * 4 + 3] = Alpha;
        }
    }
} // namespace BinImg