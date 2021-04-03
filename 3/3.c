// This program writes a simple not-small 2 page valid executable.
// The executable's code is just a breakpoint.
// It works.
// This is then a starting point for minimization.
#include <stdio.h>
#include <windows.h>

int app()
{
    return 42;
}

int getSize(const void* a)
{
    int i = 0;
    unsigned char* b = (unsigned char*)a;
    while (b[i] != 0xcc && b[i] != 0xc3) // but interior bytes...
        ++i;
    return ++i;
}

int main()
{
    int functionSize = getSize(app);
    int imageSize = 0x200 + functionSize;
    struct
    {
        IMAGE_DOS_HEADER dos;
        IMAGE_NT_HEADERS nt;
        IMAGE_SECTION_HEADER section;
        char pad[0x200 - (sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS) + sizeof(IMAGE_SECTION_HEADER))];
        char data[0x1000];
    } a;
    FILE* file = fopen("4.exe", "wb");
    IMAGE_OPTIONAL_HEADER64* opt = &a.nt.OptionalHeader;
    IMAGE_DOS_HEADER* dos = &a.dos;
    IMAGE_NT_HEADERS* nt = &a.nt;

    ZeroMemory(&a, sizeof(a));

    // msdos header
    ((char*)dos)[0] = 'M';
    ((char*)dos)[1] = 'Z';
    dos->e_lfanew = sizeof(a.dos);

    ((char*)nt)[0] = 'P';
    ((char*)nt)[1] = 'E';
    nt->FileHeader.Machine = IMAGE_FILE_MACHINE_AMD64;
    nt->FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER64);
    nt->FileHeader.NumberOfSections = 1;
    nt->FileHeader.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE;

    opt->Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
    opt->AddressOfEntryPoint = 0x1000;
    opt->SectionAlignment = 0x1000;
    opt->FileAlignment = 0x200;
    opt->SizeOfImage = 0x2000;
    opt->SizeOfHeaders = 0x200;
    opt->MajorOperatingSystemVersion = 6;
    opt->MajorSubsystemVersion = 6;
    opt->ImageBase = 0x10000;
    opt->Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;

    a.section.VirtualAddress = 0x1000;
    a.section.SizeOfRawData = functionSize;
    a.section.Misc.VirtualSize = 0x1000;
    a.section.PointerToRawData = 0x200;
    a.section.Characteristics = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;

    memcpy(a.data, app, functionSize);

    fwrite(&a, 1, imageSize, file);
}
