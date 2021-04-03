// This program writes a simple not-small 2 page valid executable.
// The executable's code is just a breakpoint.
// It works.
// This is then a starting point for minimization.
#include <stdio.h>
#include <windows.h>

int main()
{
    struct
    {
        IMAGE_DOS_HEADER dos;
        IMAGE_NT_HEADERS nt;
        IMAGE_SECTION_HEADER section;
        char pad[0x1000 - (sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_NT_HEADERS) + sizeof(IMAGE_SECTION_HEADER))];
        __declspec(align(0x1000)) char data[0x1000];
    } a;
    FILE* file = fopen("3.exe", "wb");
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
    opt->FileAlignment = 0x1000;
    opt->SizeOfImage = 0x2000;
    opt->SizeOfHeaders = 0x1000;
    opt->MajorOperatingSystemVersion = 6;
    opt->MajorSubsystemVersion = 6;
    opt->ImageBase = 0x10000;
    opt->Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
    opt->DllCharacteristics = IMAGE_DLLCHARACTERISTICS_NO_BIND;

    a.section.VirtualAddress = 0x1000;
    a.section.SizeOfRawData = 0x1000;
    a.section.Misc.VirtualSize = 0x1000;
    a.section.PointerToRawData = 0x1000;
    a.section.Characteristics = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
    a.data[0] = 0xcc; // breakpoint to start

    fwrite(&a, 1, sizeof(a), file);
}
