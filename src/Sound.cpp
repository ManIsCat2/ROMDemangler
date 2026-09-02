#include "Sound.h"

struct ALSeqFile {
    u32 Offset = 0;
    u16 Revision = 0;
    u16 SeqCount = 0;
};

static ALSeqFile *SeqFileHeader;
std::set<u8> SequenceMusics = {1, 2, 11, 13, 14, 15, 16, 18, 20, 21, 22, 23, 27, 28, 29, 30, 31, 32, 33};
std::vector<std::string> SequenceNames;

ALSeqFile *FindSeqFileHeader(N64Rom &Rom) {
    std::vector<ALSeqFile> ALSeqFiles;

    for (u32 I = 0x100000; I < Rom.Size - 16; I += 16) {
        u16 Revision = Rom.ReadBytesPhysical<u16>(I);
        u16 SeqCount = Rom.ReadBytesPhysical<u16>(I + 2);
        if (SeqCount == 0 || SeqCount > 512) continue;

        u32 HeaderEnd = 4 + SeqCount * 8;
        u32 Offset0 = Rom.ReadBytesPhysical<u32>(I + 4);

        bool FirstOK = Offset0 == HeaderEnd || Offset0 == ((HeaderEnd + 15) & ~15);
        if (!FirstOK) continue;

        bool Valid = true;

        //u32 LastOffset = Offset0;

        for (u32 S = 0; S < SeqCount; S++) {
            u32 Off = Rom.ReadBytesPhysical<u32>(I + 4 + S * 8);
            u32 Len = Rom.ReadBytesPhysical<u32>(I + 8 + S * 8);

            if (Len == 0) {
                Valid = false;
                break;
            }

            if (Off < HeaderEnd) {
                Valid = false;
                break;
            }

            if (Off & 0xF) {
                Valid = false;
                break;
            }

            /*if (S) {
                if (Off < LastOffset) {
                    Valid = false;
                    break;
                }
            }*/

            //LastOffset = Off;
        }

        if (!Valid) continue;

        ALSeqFiles.push_back({ I, Revision, SeqCount });
    }

    if (ALSeqFiles.empty()) {
        return nullptr;
    }

    std::sort(ALSeqFiles.begin(), ALSeqFiles.end(),
        [](const ALSeqFile& a, const ALSeqFile& b) {
            return a.SeqCount > b.SeqCount;
        });

    printf("Found ALSeqFile headers:\n");
    for (const auto &SeqFile : ALSeqFiles) {
        printf("  0x%08x  (count: %u, revision: %u)\n", SeqFile.Offset, SeqFile.SeqCount, SeqFile.Revision);
    }

    printf("Selected: 0x%08x\n", ALSeqFiles.front().Offset);

    static ALSeqFile Result;
    Result = ALSeqFiles.front();
    return &Result;
}

void ExportSequence(N64Rom &Rom, u8 SeqID, const char *FilePath) {
    FILE *SeqDump = fopen(FilePath, "wb");

    u32 SeqFileOffset = SeqFileHeader->Offset + 4 + (SeqID * 8);
    u32 SeqOffset = Rom.ReadBytesPhysical<u32>(SeqFileOffset);
    u32 SeqLength = Rom.ReadBytesPhysical<u32>(SeqFileOffset + 4);

    if (SeqLength == 0) {
        printf("Sequence %d has a length of 0, Skipping\n", SeqID);
        fclose(SeqDump);
        return;
    }

    std::vector<u8> M64Data(SeqLength);
    Rom.ReadBytesPtr<u8>(SeqFileHeader->Offset + SeqOffset, M64Data.data(), SeqLength);

    fwrite(M64Data.data(), 1, SeqLength, SeqDump);
    fclose(SeqDump);
}

void ExportSequences(N64Rom &Rom) {
    std::string MusicPath = "output/sound";
    fs::create_directories(MusicPath);

    printf("Exporting sequences\n");

    SeqFileHeader = FindSeqFileHeader(Rom);
    if (!SeqFileHeader) {
        printf("No ALSeqFile could be found.\n");
        return;
    }
    if (GameType.IsOldBinary()) {
        GetSequenceNames(Rom);
    }

    bool UseNames = !SequenceNames.empty();

    for (auto &I : SequenceMusics) {
        if (SeqFileHeader->SeqCount <= I) {
            printf("Sequence 0x%02x is out of bounds, Skipping\n", I);
            continue;
        }
        if (UseNames && I < SequenceNames.size()) {
            std::string Name = SequenceNames[I];
            std::string FilePath = MusicPath + "/" + Name + ".m64";
            ExportSequence(Rom, I, FilePath.c_str());
            continue;
        }
        char FileName[256];
        sprintf(FileName, "seq_0x%02x.m64", I);
        std::string FilePath = MusicPath + "/" + std::string(FileName);

        ExportSequence(Rom, I, FilePath.c_str());
    }
}

void GetSequenceNames(N64Rom &Rom) {
    u32 Entry = 0x7F1000;
    if (GameType.GetID() == GT_EDITOR) {
        Entry = 0x0081D000;
    }
    for (int I = 0; I < SeqFileHeader->SeqCount; I++) {
        u8 NameLen = Rom.ReadBytesPhysical<u8>(Entry);
        
        if (NameLen == 0xFF) {
            break;
        }

        std::string Name;
        if (NameLen > 0) {
            Name.resize(NameLen);
            Rom.ReadBytesPtr<char>(Entry + 1, &Name[0], NameLen);
        }
        SequenceNames.push_back(Name);

        Entry += 1 + NameLen;
    }
}

u8 GetSeqNInst(N64Rom &Rom, u8 SeqID) {
    u32 SeqMagic = (GameType.IsDecomp()) ? 0x7b0800 : 0x7f0000;

    u32 Entry = SeqMagic + SeqID * 2;

    u16 BankOff = Rom.ReadBytesPhysical<u16>(Entry);
    u32 BankByteOff = SeqMagic + (u32)(BankOff) + 1;
    u8 Bank = Rom.ReadBytesPhysical<u8>(BankByteOff);

    return Bank;
}
