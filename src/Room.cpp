#include "Room.h"

void ExportRoom(N64Rom &Rom, u8 Area, const std::string &LvlName, u32 Entry, LevelScript &Script, const char *FilePath) {
    if (!Entry) return;
    FILE *RoomDump = fopen(FilePath, "w");

    u16 Len = Script.AreaDatas[Area].TriangleCount;

    fprintf(RoomDump, "const u8 %s_%u_rooms_0x%x[] = {\n", LvlName.c_str(), Area, Entry);

    for (s32 I = 0; I < Len; I++) {
        if (I % 16 == 0) {
            fprintf(RoomDump, "    ");
        }
        fprintf(RoomDump, "%u, ", Rom.ReadBytes<u8>(Entry + I));
        if (I % 16 == 15) {
            fprintf(RoomDump, "\n");
        }
    }

    fprintf(RoomDump, "\n};\n");
    fclose(RoomDump);
}