#include <stdio.h>
#include <windows.h>

extern char app[1];
extern char app_end[1];
extern int call_WriteFile[];

int main()
{
    IMAGE_NT_HEADERS64* nt = 0;
    IMAGE_OPTIONAL_HEADER64* opt = 0;
    IMAGE_SECTION_HEADER* section = 0;
    int functionSize = app_end - app;
    int imageSize = 0x200 + functionSize;
    typedef struct A
    {
        IMAGE_DOS_HEADER dos;
        IMAGE_NT_HEADERS64 nt;
        IMAGE_SECTION_HEADER section;
        char pad[0x200 - sizeof(IMAGE_DOS_HEADER) - sizeof(IMAGE_NT_HEADERS64) - sizeof(IMAGE_SECTION_HEADER)];
        char strWriteFile[sizeof("WriteFile") + 2];
        char strKernel32[sizeof("kernel32")];
        void* pfnWriteFile;
        void* pfn0;
        IMAGE_IMPORT_DESCRIPTOR importKernel32;
        IMAGE_IMPORT_DESCRIPTOR import0;
        char app[0x1000];
    } A;
    A a;
    typedef void (*APP)(void);
    ZeroMemory(&a, sizeof(a));
    IMAGE_DOS_HEADER* dos = &a.dos;

    // PE and DOS can overlap.
    dos->e_lfanew = sizeof(*dos);
    nt = (IMAGE_NT_HEADERS64*)(dos->e_lfanew + (char*)dos);
    opt = &nt->OptionalHeader;

    nt->FileHeader.SizeOfOptionalHeader = sizeof(*opt); // possible limit, not understood (optional and PE overlap)
    section = (IMAGE_SECTION_HEADER*)(nt->FileHeader.SizeOfOptionalHeader + (char*)opt);

    // msdos header
    ((char*)dos)[0] = 'M';
    ((char*)dos)[1] = 'Z';

    // PE header
    ((char*)nt)[0] = 'P';
    ((char*)nt)[1] = 'E';
    nt->FileHeader.Machine = IMAGE_FILE_MACHINE_AMD64;
    nt->FileHeader.NumberOfSections = 1;
    nt->FileHeader.Characteristics = IMAGE_FILE_EXECUTABLE_IMAGE;

    opt->Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
    opt->AddressOfEntryPoint = 0x1000;
    opt->SectionAlignment = 0x1000;
    opt->FileAlignment = 0x200;
    opt->SizeOfImage = 0x2000;

    // Requirements on SizeOfHeaders are not understood.
    opt->SizeOfHeaders = 0x200;
    opt->MajorOperatingSystemVersion = 6;
    opt->MajorSubsystemVersion = 6;
    opt->ImageBase = 0x10000;
    opt->Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;

    section->VirtualAddress = 0x1000;
    section->SizeOfRawData = functionSize;
    section->Misc.VirtualSize = 0x1000;
    section->PointerToRawData = 0x200;
    section->Characteristics = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;

    ((APP)app)();

    memcpy(a.app, app, functionSize); // dos can work too but no point really

    if (IsDebuggerPresent()) __debugbreak();

    acall
    call_WriteFile[-1] = ((char*)&call_WriteFile - app) + a.app - offsetof(A, pfnWriteFile);

    opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = offsetof(A, importKernel32) - 0x200 + 0x1000;
    opt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = 2 * sizeof(IMAGE_IMPORT_DESCRIPTOR); // ?
    a.pfnWriteFile = offsetof(A, strWriteFile) - 0x200 + 0x1000;
    a.importKernel32.OriginalFirstThunk = offsetof(A, pfnWriteFile) - 0x200 + 0x1000;
    a.importKernel32.FirstThunk = offsetof(A, pfnWriteFile) - 0x200 + 0x1000;
    a.importKernel32.Name = offseof(A, strKernel32) - 0x200 + 0x1000;
    strcpy(a.strKernel32, "kernel32");
    strcpy(a.strWriteFile, "\1\1WriteFile"); // starts with hint

    fwrite(&a, 1, imageSize, fopen("9.exe", "wb"));
}
