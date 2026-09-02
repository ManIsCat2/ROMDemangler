#include "MovingTexture.h"
#include "LevelScript.h"

std::vector<MovingTextureQC> MovingTextures = {};

// this is the most depressing function i have ever wrote
void ExportMovTex(N64Rom &Rom, u8 Area, const std::string &LvlName, LevelScript &Script, const char *FilePath) {
    FILE *MovTexDump = fopen(FilePath, "w");
    u8 WaterCount = Script.AreaDatas[Area].WaterBoxCount;
    if (WaterCount == 0) {
        fclose(MovTexDump);
        return;
    }
    for (u32 DT = 0; DT < Script.AreaDatas[Area].WaterBoxParams.size(); DT++) {
        s16 WaterType = Script.AreaDatas[Area].WaterBoxParams[DT] & 0xff;
        u32 WaterBox = 0;
        std::vector<u32> MovTexPtrs = {};
        std::vector<std::string> MovTexQCStrings = {};

        if (GameType.GetID() == GT_EDITOR) {
            WaterBox = (0x19001800+0x50*WaterType);
        } else {
            WaterBox = (0x19006000+0x280*WaterType+0x50*Area);
        }
        
        for (u32 W = 0; W < WaterCount; W++) {
            u32 Ptr = Rom.ReadBytes<u32>(WaterBox + 4 + W * 8);
            if (Ptr == 0) break;
            MovTexPtrs.push_back((Ptr));
        }

        u32 MovTexID = 0;
        for (u32 MT = 0; MT < MovTexPtrs.size(); MT++) {
            u32 Entry = MovTexPtrs[MT];
            char MovTexStr[1024];
            snprintf(MovTexStr, 1024, "%s_%u_movtex_%u_%u", LvlName.c_str(), Area, DT, MovTexID);
            fprintf(MovTexDump, "static Movtex %s[] = {", MovTexStr);
            MovTexQCStrings.push_back(MovTexStr);

            for (u32 WD = 0; WD < 0x20; WD+=2) {
                fprintf(MovTexDump, "%d",
                Rom.ReadBytes<s16>(Entry + WD));
                if (WD < 30) fprintf(MovTexDump, ", ");
            }
            fprintf(MovTexDump, "};\n");
            MovTexID++;
        }
        fprintf(MovTexDump, "const struct MovtexQuadCollection %s_%u_movtex_%u[] = {\n", LvlName.c_str(), Area, DT);
        for (u32 MSTR = 0; MSTR < MovTexQCStrings.size(); MSTR++) {
            fprintf(MovTexDump, "    {%u, %s},\n", MSTR, MovTexQCStrings[MSTR].c_str());
        }
        fprintf(MovTexDump, "    {-1, NULL},\n");
        fprintf(MovTexDump, "};\n");

        MovingTextureQC NewMovTexQC;
        NewMovTexQC.LvlName = LvlName;
        NewMovTexQC.LvlID = Script.LevelID;
        NewMovTexQC.Area = Area;
        NewMovTexQC.Type = WaterType;
        NewMovTexQC.Index = DT;
        MovingTextures.push_back(NewMovTexQC);
    }
    fclose(MovTexDump);
}