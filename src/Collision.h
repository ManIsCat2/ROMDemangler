#pragma once

#include "LevelScript.h"

extern void ExportCollision(N64Rom &Rom, u8 Area, const std::string &LvlName, u32 SegAddr, LevelScript &Script, const char *FilePath, bool IsActor = false);