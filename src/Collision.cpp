#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>

#include "Collision.h"

struct Vertex3D {
    s16 X, Y, Z;
};

struct FloatVertex3D {
    f32 X, Y, Z;
};

static bool ColsHaveExtraForce = false;

f32 CalculateTriangleArea(const FloatVertex3D &V1, const FloatVertex3D &V2, const FloatVertex3D &V3) {
    auto Distance = [](const FloatVertex3D &A, const FloatVertex3D &B) {
        f32 Dx = B.X - A.X;
        f32 Dy = B.Y - A.Y;
        f32 Dz = B.Z - A.Z;
        return std::sqrt(Dx * Dx + Dy * Dy + Dz * Dz);
    };
    f32 S1 = Distance(V1, V2);
    f32 S2 = Distance(V2, V3);
    f32 S3 = Distance(V3, V1);
    f32 S = (S1 + S2 + S3) / 2.0f;
    f32 AreaSq = S * (S - S1) * (S - S2) * (S - S3);
    return (AreaSq > 0.0f) ? std::sqrt(AreaSq) : 0.0f;
}

void ExportCollision(N64Rom &Rom, u8 Area, const std::string &LvlName, u32 SegAddr, LevelScript &Script, const char *FilePath) {
Retry:
    FILE *ColDump = fopen(FilePath, "w");
    fprintf(ColDump, "const Collision %s_area_%u_collision_0x%x[] = {\n", LvlName.c_str(), Area, SegAddr);
    fprintf(ColDump, "    COL_INIT(),\n");

    u32 Entry = SegAddr;

    s16 NumVerts = Rom.ReadBytes<s16>(Entry + 2);
    Entry += 4;
    
    u32 VertexStart = Entry;

    std::vector<Vertex3D> Vertices;
    for (s32 V = 0; V < NumVerts; V++) {
        s16 X = Rom.ReadBytes<s16>(Entry + V * 6 + 0);
        s16 Y = Rom.ReadBytes<s16>(Entry + V * 6 + 2);
        s16 Z = Rom.ReadBytes<s16>(Entry + V * 6 + 4);
        Vertices.push_back({X, Y, Z});
    }

    Entry += NumVerts * 6;
    s32 XOffset = 0;
    s32 Guard = 0;
    const std::set<s16> SpecialTris = {0x0E, 0x24, 0x25, 0x27, 0x2C, 0x2D};

    std::map<s16, std::vector<std::vector<s16>>> SurfaceTris;
    std::vector<Vertex3D> DeathPlaneVertices;

    while (true) {
        s16 SurfType = Rom.ReadBytes<s16>(Entry + XOffset + 0);
        s16 NumTris = Rom.ReadBytes<s16>(Entry + XOffset + 2);
        if (SurfType == 0x41 || Guard > 50000) {
            if (Guard > 50000) {
                if (!ColsHaveExtraForce) {
                    printf("Level has broken collision data, retrying with extra fields\n");
                    ColsHaveExtraForce = true;
                    fclose(ColDump);
                    goto Retry;
                }
            }
            break;
        }

        if (SurfaceTris.find(SurfType) == SurfaceTris.end()) {
            SurfaceTris[SurfType] = std::vector<std::vector<s16>>();
        }

        if (SpecialTris.count(SurfType) || ColsHaveExtraForce) {
            for (s32 T = 0; T < NumTris; T++) {
                s16 V1 = Rom.ReadBytes<s16>(Entry + XOffset + 4 + T * 8 + 0);
                s16 V2 = Rom.ReadBytes<s16>(Entry + XOffset + 4 + T * 8 + 2);
                s16 V3 = Rom.ReadBytes<s16>(Entry + XOffset + 4 + T * 8 + 4);
                s16 Param = Rom.ReadBytes<s16>(Entry + XOffset + 4 + T * 8 + 6);
                SurfaceTris[SurfType].push_back({V1, V2, V3, Param});
            }
            XOffset += NumTris * 8 + 4;
        } else {
            for (s32 T = 0; T < NumTris; T++) {
                s16 V1 = Rom.ReadBytes<s16>(Entry + XOffset + 4 + T * 6 + 0);
                s16 V2 = Rom.ReadBytes<s16>(Entry + XOffset + 4 + T * 6 + 2);
                s16 V3 = Rom.ReadBytes<s16>(Entry + XOffset + 4 + T * 6 + 4);
                
                SurfaceTris[SurfType].push_back({V1, V2, V3});
            }
            XOffset += NumTris * 6 + 4;
        }
        Guard++;
    }

    // there are NO comments here whatsoever
    // i cant even read it myslf (i wrote it)
    if (CollisionFix && SurfaceTris.find(10) != SurfaceTris.end()) {
        std::vector<std::vector<s16>> NewTriangles;
        s32 Offset = Vertices.size();

        for (const auto &Tri : SurfaceTris[10]) {
            FloatVertex3D Fv1 = {(f32)Vertices[Tri[0]].X, (f32)Vertices[Tri[0]].Y, (f32)Vertices[Tri[0]].Z};
            FloatVertex3D Fv2 = {(f32)Vertices[Tri[1]].X, (f32)Vertices[Tri[1]].Y, (f32)Vertices[Tri[1]].Z};
            FloatVertex3D Fv3 = {(f32)Vertices[Tri[2]].X, (f32)Vertices[Tri[2]].Y, (f32)Vertices[Tri[2]].Z};

            f32 Area = CalculateTriangleArea(Fv1, Fv2, Fv3);
            if (Area < 268419072.0f) {
                NewTriangles.push_back(Tri);
                continue;
            }

            std::vector<Vertex3D> NewVertsList;
            FloatVertex3D Edges[3][2] = { {Fv1, Fv2}, {Fv2, Fv3}, {Fv3, Fv1} };

            auto CrossQuadrant = [](const FloatVertex3D &E1, const FloatVertex3D &E2, int Axis) {
                f32 Val1 = (Axis == 0) ? E1.X : E1.Z;
                f32 Val2 = (Axis == 0) ? E2.X : E2.Z;
                return (Val1 > 0.0f && Val2 < 0.0f) || (Val1 < 0.0f && Val2 > 0.0f);
            };

            auto CondAppend = [](std::vector<Vertex3D> &Arr, const Vertex3D &Item) {
                if (std::find_if(Arr.begin(), Arr.end(), [&](const Vertex3D &V) {
                    return V.X == Item.X && V.Y == Item.Y && V.Z == Item.Z;
                }) == Arr.end()) {
                    Arr.push_back(Item);
                }
            };

            for (int EIdx = 0; EIdx < 3; EIdx++) {
                if (CrossQuadrant(Edges[EIdx][0], Edges[EIdx][1], 0)) {
                    for (int I = 0; I < 2; I++) {
                        FloatVertex3D V = Edges[EIdx][I];
                        FloatVertex3D One = Edges[EIdx][(I + 1) % 2];
                        f32 LerpZ = 0.0f;
                        if ((One.X - V.X) != 0.0f) {
                            LerpZ = (V.Z * (One.X - 0.0f) + One.Z * (0.0f - V.X)) / (One.X - V.X);
                        }
                        if (V.X > 0.0f) {
                            CondAppend(NewVertsList, {(s16)V.X, (s16)V.Y, (s16)V.Z});
                        } else {
                            CondAppend(NewVertsList, {0, (s16)V.Y, (s16)LerpZ});
                        }
                        if (V.X < 0.0f) {
                            CondAppend(NewVertsList, {(s16)V.X, (s16)V.Y, (s16)V.Z});
                        } else {
                            CondAppend(NewVertsList, {0, (s16)V.Y, (s16)LerpZ});
                        }
                    }
                }
                if (CrossQuadrant(Edges[EIdx][0], Edges[EIdx][1], 2)) {
                    for (int I = 0; I < 2; I++) {
                        FloatVertex3D V = Edges[EIdx][I];
                        FloatVertex3D One = Edges[EIdx][(I + 1) % 2];
                        f32 LerpX = 0.0f;
                        if ((One.Z - V.Z) != 0.0f) {
                            LerpX = (V.X * (One.Z - 0.0f) + One.X * (0.0f - V.Z)) / (One.Z - V.Z);
                        }
                        if (V.Z > 0.0f) {
                            CondAppend(NewVertsList, {(s16)V.X, (s16)V.Y, (s16)V.Z});
                        } else {
                            CondAppend(NewVertsList, {(s16)LerpX, (s16)V.Y, 0});
                        }
                        if (V.Z < 0.0f) {
                            CondAppend(NewVertsList, {(s16)V.X, (s16)V.Y, (s16)V.Z});
                        } else {
                            CondAppend(NewVertsList, {(s16)LerpX, (s16)V.Y, 0});
                        }
                    }
                }
            }

            if (!NewVertsList.empty()) {
                f32 CentroidX = 0.0f, CentroidZ = 0.0f;
                for (const auto &Nv : NewVertsList) {
                    CentroidX += Nv.X;
                    CentroidZ += Nv.Z;
                }
                CentroidX /= NewVertsList.size();
                CentroidZ /= NewVertsList.size();

                std::sort(NewVertsList.begin(), NewVertsList.end(), [CentroidX, CentroidZ](const Vertex3D &A, const Vertex3D &B) {
                    f32 AngleA = std::atan2((f32)A.Z - CentroidZ, (f32)A.X - CentroidX);
                    f32 AngleB = std::atan2((f32)B.Z - CentroidZ, (f32)B.X - CentroidX);
                    return AngleA < AngleB;
                });

                for (const auto &Nv : NewVertsList) {
                    DeathPlaneVertices.push_back(Nv);
                }

                if (NewVertsList.size() >= 3) {
                    for (size_t I = 1; I + 1 < NewVertsList.size(); I++) {
                        s16 Idx1 = (s16)(Offset);
                        s16 Idx2 = (s16)(Offset + I);
                        s16 Idx3 = (s16)(Offset + I + 1);

                        FloatVertex3D Nv1 = {(f32)NewVertsList[0].X, (f32)NewVertsList[0].Y, (f32)NewVertsList[0].Z};
                        FloatVertex3D Nv2 = {(f32)NewVertsList[I].X, (f32)NewVertsList[I].Y, (f32)NewVertsList[I].Z};
                        FloatVertex3D Nv3 = {(f32)NewVertsList[I+1].X, (f32)NewVertsList[I+1].Y, (f32)NewVertsList[I+1].Z};
                        f32 Ndx1 = Nv2.X - Nv1.X;
                        f32 Ndz1 = Nv2.Z - Nv1.Z;
                        f32 Ndx2 = Nv3.X - Nv1.X;
                        f32 Ndz2 = Nv3.Z - Nv1.Z;
                        f32 NcrossY = (Ndz1 * Ndx2) - (Ndx1 * Ndz2);

                        if (NcrossY < 0.0f) {
                            NewTriangles.push_back({Idx1, Idx3, Idx2});
                        } else {
                            NewTriangles.push_back({Idx1, Idx2, Idx3});
                        }
                    }
                }
                Offset += NewVertsList.size();
            } else {
                NewTriangles.push_back(Tri);
            }
        }
        SurfaceTris[10] = NewTriangles;
    }

    s32 TotalVertices = Vertices.size() + DeathPlaneVertices.size();
    fprintf(ColDump, "    COL_VERTEX_INIT(%d),\n", TotalVertices);
    for (const auto &V : Vertices) {
        fprintf(ColDump, "    COL_VERTEX(%d, %d, %d),\n", V.X, V.Y, V.Z);
    }
    for (const auto &V : DeathPlaneVertices) {
        fprintf(ColDump, "    COL_VERTEX(%d, %d, %d),\n", V.X, V.Y, V.Z);
    }

    for (const auto &Pair : SurfaceTris) {
        s16 Type = Pair.first;
        const auto &TrisList = Pair.second;
        fprintf(ColDump, "    COL_TRI_INIT(%d, %d),\n", Type, (int)TrisList.size());
        for (const auto &Tri : TrisList) {
            if (SpecialTris.count(Type)) {
                fprintf(ColDump, "    COL_TRI_SPECIAL(%d, %d, %d, %d),\n", Tri[0], Tri[1], Tri[2], Tri[3]);
            } else {
                fprintf(ColDump, "    COL_TRI(%d, %d, %d),\n", Tri[0], Tri[1], Tri[2]);
            }
        }
    }
    fprintf(ColDump, "    COL_TRI_STOP(),\n");

    Guard = 0;
    XOffset += 2;
    while (true) {
        s16 SpecialType = Rom.ReadBytes<s16>(Entry + XOffset + 0);
        s16 Stuff = Rom.ReadBytes<s16>(Entry + XOffset + 2);
        if (SpecialType == 0x42 || Guard > 50000) {
            if (Guard > 50000) {
                printf("Level has broken collision data, stopping collision export\n");
            }
            fprintf(ColDump, "    COL_END(),\n");
            break;
        } else if (SpecialType == 0x44) {
            fprintf(ColDump, "    COL_WATER_BOX_INIT(%d),\n", Stuff);
            Script.AreaDatas[Area].WaterBoxCount = Stuff;
            XOffset += 4;
            for (s32 W = 0; W < Stuff; W++) {
                s16 ID = Rom.ReadBytes<s16>(Entry + XOffset + 0);
                s16 X1 = Rom.ReadBytes<s16>(Entry + XOffset + 2);
                s16 Z1 = Rom.ReadBytes<s16>(Entry + XOffset + 4);
                s16 X2 = Rom.ReadBytes<s16>(Entry + XOffset + 6);
                s16 Z2 = Rom.ReadBytes<s16>(Entry + XOffset + 8);
                s16 Y = Rom.ReadBytes<s16>(Entry + XOffset + 10);
                fprintf(ColDump, "    COL_WATER_BOX(%d, %d, %d, %d, %d, %d),\n", ID, X1, Z1, X2, Z2, Y);
                XOffset += 12;
            }
        } else {
            fprintf(ColDump, "    COL_END(),\n");
            break;
        }
        Guard++;
    }

    fprintf(ColDump, "};\n");
    fclose(ColDump);
}