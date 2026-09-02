#pragma once

#include "LevelScript.h"

extern void ExportRoom(N64Rom &Rom, u8 Area, const std::string &LvlName, u32 Entry, LevelScript &Script, const char *FilePath);