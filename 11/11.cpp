// This takes the learnings of 10.cpp and produces
// the old "return 42" case (case 7).
// Same functionality, same size, but less trial and error.

#include <stdio.h>
#include <windows.h>

int app(void)
{
    return 42;
}

int getSize(const void* a)
{
    int i = 0;
    unsigned char* b = (unsigned char*)a;
    while (b[i] != 0xc3) // but interior bytes...
        ++i;
    return ++i;
}

int main()
{
    int functionSize = getSize(app);
    IMAGE_DOS_HEADER dos = {IMAGE_DOS_SIGNATURE};
    dos.e_lfanew = 16;

    IMAGE_NT_HEADERS64 nt =
    {
        IMAGE_NT_SIGNATURE, // 4
        {
            IMAGE_FILE_MACHINE_AMD64, // 2 bytes
            1, // number of sections  // 2 bytes
            0, // time                  // 4 bytes
            0, // symbols (for obj files) // 4 bytes
            0, // symbols (for obj files) // 4 bytes
            offsetof(IMAGE_OPTIONAL_HEADER64, DataDirectory), // 2 bytes
            IMAGE_FILE_EXECUTABLE_IMAGE, // value=2 not an obj, required
        },
        {
            IMAGE_NT_OPTIONAL_HDR64_MAGIC,
            0, // MajorLinkerVersion
            0, // MinorLinkerVersion
            0, // SizeOfCode
            0, // SizeOfInitializedData
            0, // SizeOfUninitializedData
            0, // AddressOfEntryPoint
            0, // BaseOfCode
            0, // ImageBase
            0x1000, // SectionAlignment
            0x1000, // FileAlignment
            0, // MajorOperatingSystemVersion
            0, // MinorOperatingSystemVersion 
            0, // MajorImageVersion
            0, // MinorImageVersion
            3, // MajorSubsystemVersion
            10, // MinorSubsystemVersion
            0, // Win32VersionValue
            0x2000, // SizeOfImage
            16 // offsetOfPe
            + 0//4 // pe
            + 0//sizeof(IMAGE_FILE_HEADER)
            + 0x40,//offsetof(IMAGE_OPTIONAL_HEADER64, DataDirectory), // SizeOfHeaders // constraints on this are not understood
            0, // checksum
            IMAGE_SUBSYSTEM_WINDOWS_CUI, // subsytem
            0, // DllCharacteristics
            0, // SizeOfStackReserve
            0, // SizeOfStackCommit
            0, // SizeOfHeapReserve
            0, // SizeOfHeapCommit
            0, // LoaderFlags
            0, // number of data directories
        }
    };

    IMAGE_SECTION_HEADER section =
    { // section
        { 0 }, // name
        { 0x1000 }, // Misc.VirtualSize
        0x1000, // VirtualAddress
        1, // SizeOfRawData constraints on this are not understood
        1, // PointerToRawData // presumably rounded down to 0
        0, // relocs (obj only)
        0, // lines (obj only)
        0, // relocs (obj only)
        0, // lines (obj only)
        IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE, // Characteristics
    };

    // reuse unused fields
    IMAGE_SECTION_HEADER* psection = (IMAGE_SECTION_HEADER*)&nt.OptionalHeader.DataDirectory;

    *psection = section;

    memcpy(psection + 1, app, functionSize);

    nt.OptionalHeader.AddressOfEntryPoint = (char*)(psection + 1) - (char*)&nt + dos.e_lfanew + 0x1000;

    FILE* file = fopen("12.exe", "wb");

    fseek(file, 0, SEEK_SET);
    fwrite(&dos, 1, sizeof(dos), file);

    fseek(file, dos.e_lfanew, SEEK_SET);
    fwrite(&nt, 1, sizeof(nt), file);

    fseek(file, 60, SEEK_SET);
    fwrite(&dos.e_lfanew, 1, sizeof(dos.e_lfanew), file);

    fclose(file);

    system("dir 12.exe");
    HMODULE h = LoadLibrary("12.exe");
    int err = h ? 0 : GetLastError();
    printf("LoadLibrary:%p err:%d\n", h, err);
    if (h)
    {
        printf("loadable\n");
    }
    else
    {
        printf("not loadable\n");
        return err;
    }
}
