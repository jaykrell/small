// This writes a simple valid no tricks 1k .exe.
//
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
    struct
    {
        IMAGE_DOS_HEADER dos;
        IMAGE_NT_HEADERS32 nt;
        IMAGE_SECTION_HEADER section;
        char pad[0x200 - sizeof(IMAGE_DOS_HEADER) - sizeof(IMAGE_NT_HEADERS32) - sizeof(IMAGE_SECTION_HEADER)];
        char data[0x200];
    } a;
    ZeroMemory(&a, sizeof(a));
    FILE* file = fopen("6.exe", "wb");
    IMAGE_DOS_HEADER* dos = &a.dos;
    IMAGE_NT_HEADERS32* nt = &a.nt;
    IMAGE_OPTIONAL_HEADER32* opt = &nt->OptionalHeader;
    IMAGE_SECTION_HEADER* section = &a.section;
    int imageSize = sizeof(a);

    // msdos header
    ((char*)dos)[0] = 'M';
    ((char*)dos)[1] = 'Z';
    dos->e_lfanew = sizeof(*dos);

    // PE header
    ((char*)nt)[0] = 'P';
    ((char*)nt)[1] = 'E';
    nt->FileHeader.Machine = IMAGE_FILE_MACHINE_I386;
    nt->FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER32);
    nt->FileHeader.NumberOfSections = 1;
    nt->FileHeader.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE;

    opt->Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
    opt->AddressOfEntryPoint = 0x1000;
    opt->SectionAlignment = 0x1000;
    opt->FileAlignment = 0x200;
    opt->SizeOfImage = 0x2000;

    opt->SizeOfHeaders = 0x200;
    opt->MajorOperatingSystemVersion = 6;
    opt->MajorSubsystemVersion = 6;
    opt->ImageBase = 0x10000;
    opt->Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;

    section->VirtualAddress = 0x1000;
    section->SizeOfRawData = 0x200;
    section->Misc.VirtualSize = 0x1000;
    section->PointerToRawData = 0x200;
    section->Characteristics = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;

    memcpy(a.data, app, functionSize);

    fwrite(&a, 1, imageSize, file);
}
