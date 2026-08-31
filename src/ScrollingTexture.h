#pragma once

#include "LevelScript.h"

extern std::vector<ScrollTexture> ScrollingTextures;

extern u16 GetScrollAxis(u16 Dir);
extern u16 GetScrollType(u16 Dir);
extern ScrollTexture ConvertRMTexScrolls(LevelScript &Script, u32 Bparam, u16 NumVtx, u16 Dir, s16 Speed);
extern ScrollTexture ConvertEditorTexScrolls(LevelScript &Script, u32 Bparam, s16 PosX, s16 PosY, s16 PosZ, std::string &BhvName, N64Rom& Rom);
extern void ResolveScrollTargets(LevelScript &Script);