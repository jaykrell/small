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
    IMAGE_NT_HEADERS32* nt = 0;
    IMAGE_OPTIONAL_HEADER32* opt = 0;
    IMAGE_SECTION_HEADER* section = 0;
    int imageSize = -1;
    int functionSize = getSize(app);
    typedef struct A
    {
        IMAGE_DOS_HEADER dos;
        IMAGE_NT_HEADERS32 nt;
        IMAGE_SECTION_HEADER section;
        char pad[0x200 - sizeof(IMAGE_DOS_HEADER) - sizeof(IMAGE_NT_HEADERS32) - sizeof(IMAGE_SECTION_HEADER)];
        char data[0x200];
    } A;
    A a;
    ZeroMemory(&a, sizeof(a));
    IMAGE_DOS_HEADER* dos = &a.dos;

    // PE and DOS can overlap.
    dos->e_lfanew = 16;
    nt = (IMAGE_NT_HEADERS32*)(dos->e_lfanew + (char*)dos);
    opt = &nt->OptionalHeader;

    // Most data directories are not needed. Truncate optional header. [1] or [0]
    // ought to work here. [7] is experimentally found and makes no sense.
    nt->FileHeader.SizeOfOptionalHeader = offsetof(IMAGE_OPTIONAL_HEADER32, DataDirectory[7]);
    imageSize = sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS32) + sizeof(IMAGE_SECTION_HEADER)
         - sizeof(IMAGE_OPTIONAL_HEADER32) + nt->FileHeader.SizeOfOptionalHeader;
    section = (IMAGE_SECTION_HEADER*)(nt->FileHeader.SizeOfOptionalHeader + (char*)opt);

    // msdos header
    ((char*)dos)[0] = 'M';
    ((char*)dos)[1] = 'Z';

    // PE header
    ((char*)nt)[0] = 'P';
    ((char*)nt)[1] = 'E';
    nt->FileHeader.Machine = IMAGE_FILE_MACHINE_I386;
    nt->FileHeader.NumberOfSections = 1;
    nt->FileHeader.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE;

    opt->Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
    opt->AddressOfEntryPoint = 0x2;
    opt->SectionAlignment = 0x1000;
    opt->FileAlignment = 0x200;
    opt->SizeOfImage = 0x2000;

    opt->SizeOfHeaders = 0x60; // Requirements here are not understood.
    opt->MajorOperatingSystemVersion = 6;
    opt->MajorSubsystemVersion = 6;
    opt->ImageBase = 0x10000;
    opt->Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;

    section->VirtualAddress = 0x1000;
    section->SizeOfRawData = 1;
    section->Misc.VirtualSize = 0x1000;
    section->PointerToRawData = 1;
    //section->Characteristics = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;

    memcpy(2 + (char*)dos, app, functionSize); // dos can work too but no point really

    fwrite(&a, 1, imageSize, fopen("7.exe", "wb"));
}
