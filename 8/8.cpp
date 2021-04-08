#include <stdio.h>
#include <windows.h>
#include <stddef.h>

extern "C"
{
extern char app[1];
extern char app_end[1];
extern int call_WriteFile[];
}

#define FILE_ALIGN 0x200
#define ALIGN      0x1000
int main()
{
    struct A
    {
        IMAGE_DOS_HEADER dos;
        IMAGE_NT_HEADERS64 nt;
        IMAGE_SECTION_HEADER section;
        char pad[FILE_ALIGN - sizeof(IMAGE_DOS_HEADER) - sizeof(IMAGE_NT_HEADERS64) - sizeof(IMAGE_SECTION_HEADER)];
        char strWriteFile[sizeof("\0\0WriteFile")]; // starts with by 2 byte hint
        char strKernel32[sizeof("kernel32")];
        IMAGE_THUNK_DATA64 pfnWriteFile;
        IMAGE_THUNK_DATA64 pfn0;
        IMAGE_IMPORT_DESCRIPTOR importKernel32;
        IMAGE_IMPORT_DESCRIPTOR import0;
        char app[0x1000];
    } a =
    {
        { }, // dos
        { }, // nt
        { }, // section
        { }, // pad (todo)
        "\0\0WriteFile",
        "kernel32",
        { offsetof(A, strWriteFile) - FILE_ALIGN + ALIGN}, // pfnWriteFile
        { 0 }, // pfn0
        // importKernel
        {
            offsetof(A, pfnWriteFile) - FILE_ALIGN + ALIGN,
            0, // time
            0, // forwarder
            offsetof(A, strKernel32) - FILE_ALIGN + ALIGN,
            offsetof(A, pfnWriteFile) - FILE_ALIGN + ALIGN
        },
        { }, // import0
        { } // app
    };
    int functionSize = app_end - app;
    int imageSize = offsetof(A, app) + functionSize;
    typedef void (*APP)(void);
    IMAGE_DOS_HEADER* dos = &a.dos;

    // PE and DOS can overlap.
    dos->e_lfanew = sizeof(*dos);
    IMAGE_NT_HEADERS64* nt = (IMAGE_NT_HEADERS64*)(dos->e_lfanew + (char*)dos);
    IMAGE_OPTIONAL_HEADER64* opt = &nt->OptionalHeader;

    nt->FileHeader.SizeOfOptionalHeader = sizeof(*opt);
    IMAGE_SECTION_HEADER* section = (IMAGE_SECTION_HEADER*)(nt->FileHeader.SizeOfOptionalHeader + (char*)opt);

    // msdos header
    ((char*)dos)[0] = 'M';
    ((char*)dos)[1] = 'Z';

    // PE header
    ((char*)nt)[0] = 'P';
    ((char*)nt)[1] = 'E';
    nt->FileHeader.Machine = IMAGE_FILE_MACHINE_AMD64;
    nt->FileHeader.NumberOfSections = 1;
    nt->FileHeader.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE;

    opt->NumberOfRvaAndSizes = IMAGE_DIRECTORY_ENTRY_IMPORT + 1;
    opt->Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
    opt->AddressOfEntryPoint = offsetof(A, app) - FILE_ALIGN + ALIGN;
    opt->SectionAlignment = ALIGN;
    opt->FileAlignment = FILE_ALIGN;
    opt->SizeOfImage = 0x2000;

    // Requirements on SizeOfHeaders are not understood.
    opt->SizeOfHeaders = FILE_ALIGN;
    opt->MajorOperatingSystemVersion = 5;
    opt->MajorSubsystemVersion = 5;
    opt->ImageBase = 0x10000;
    opt->Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;

    section->VirtualAddress = ALIGN;
    section->SizeOfRawData = imageSize - FILE_ALIGN;
    section->Misc.VirtualSize = ALIGN;
    section->PointerToRawData = FILE_ALIGN;
    section->Characteristics = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;

    memcpy(a.app, app, functionSize);

    ((APP)&app)();

    //if (IsDebuggerPresent()) __debugbreak();

    int* acall = (int*)(&a.app[((char*)&call_WriteFile - app)]);
    acall[-1] = (char*)&a.pfnWriteFile - (char*)acall;

    opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = offsetof(A, importKernel32) - FILE_ALIGN + ALIGN;
    opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = 0;

    fwrite(&a, 1, imageSize, fopen("9.exe", "wb"));
}
