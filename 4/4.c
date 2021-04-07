// This program writes a small "valid" executable that returns 42.
// It works.
// File is under 512 bytes.
// It is not entirely understood why this works. A few strange
// things are done and occur.
//
// Entry point must be greater or equal than size of headers.
// Size of headers is allowed to be set as low as 0x5a.
// Entry point ends up ignored and treated as 0.
// File pointer of 0 does not work.
// File pointer of small is treated as 0.
//
// The result is that the headers are mapped at 0 and 0x1000,
// and the mapping at 0x1000 is executable, and execution is there.
// The first two bytes of the MS-DOS header are "ok" by chance, dec ebp, pop edx.
//
// AMD64 images seem to be more validated so do not work.
//
// Optional header can be shrunk a bit -- the data directories can be reduced to zero. Does not work.
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
    } a;
    // Empirically, not the entire section header is needed. I guess zero padding is ok?
    int imageSize = sizeof(a) - 24;
    FILE* file = fopen("5.exe", "wb");
    IMAGE_OPTIONAL_HEADER32* opt = &a.nt.OptionalHeader;
    IMAGE_DOS_HEADER* dos = &a.dos;
    IMAGE_NT_HEADERS32* nt = &a.nt;

    ZeroMemory(&a, sizeof(a));

    // msdos header
    ((char*)dos)[0] = 'M';
    ((char*)dos)[1] = 'Z';
    dos->e_lfanew = sizeof(a.dos);

    // PE header; can it overlap DOS?
    ((char*)nt)[0] = 'P';
    ((char*)nt)[1] = 'E';
    nt->FileHeader.Machine = IMAGE_FILE_MACHINE_I386;
    nt->FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER32);
    nt->FileHeader.NumberOfSections = 1;
    nt->FileHeader.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE;

    opt->Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
    opt->AddressOfEntryPoint = 2; // just after MZ, though 0 can work too
    opt->SectionAlignment = 0x1000;
    opt->FileAlignment = 0x200;
    opt->SizeOfImage = 0x2000;

    // Should be 0x200 but down to this is accepted.
    // Sometimes must be less than entry point, but not always.
    opt->SizeOfHeaders = 0x5A;

    opt->MajorOperatingSystemVersion = 6;
    opt->MajorSubsystemVersion = 6;
    opt->ImageBase = 0x10000;
    opt->Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;

    a.section.VirtualAddress = 0x1000;
    a.section.SizeOfRawData = 1; // not zero
    a.section.Misc.VirtualSize = 0x1000;
    a.section.PointerToRawData = 1; // not zero
    // Nice idea, but not needed
    //a.section.Characteristics = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;

    memcpy(2 + (char*)&a, app, functionSize);

    fwrite(&a, 1, imageSize, file);
}
