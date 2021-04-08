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
    char app[0x1000]; // over allocation
};

int main()
{
    int functionSize = app_end - app;
    int imageSize = offsetof(A, app) + functionSize;
    A a =
    {
        { // dos
            IMAGE_DOS_SIGNATURE,
            0, // e_cblp
            0, // e_cp
            0, // e_crlc
            0, // e_cparhdr
            0, // e_minalloc
            0, // e_maxalloc
            0, // e_ss
            0, // e_sp
            0, // e_csum
            0, // e_ip
            0, // e_cs
            0, // e_lfarlc
            0, // e_ovno
            {0}, // e_res
            0, // e_oemid
            0, // e_oeminfo
            {0}, // e_res2
            sizeof(IMAGE_DOS_HEADER)
        },
        { // nt
            IMAGE_NT_SIGNATURE,
            {
                IMAGE_FILE_MACHINE_AMD64,
                1, // number of sections
                0, // time
                0, // symbols (for obj files)
                0, // symbols (for obj files)
                sizeof(IMAGE_OPTIONAL_HEADER64),
                IMAGE_FILE_EXECUTABLE_IMAGE // not an obj
            },
            {
                IMAGE_NT_OPTIONAL_HDR64_MAGIC,
                0, // MajorLinkerVersion
                0, // MinorLinkerVersion
                0, // SizeOfCode
                0, // SizeOfInitializedData
                0, // SizeOfUninitializedData
                offsetof(A, app) - FILE_ALIGN + ALIGN, // AddressOfEntryPoint
                0, // BaseOfCode
                0x10000, // ImageBase
                ALIGN,
                FILE_ALIGN,
                5, // MajorOperatingSystemVersion
                0, // MinorOperatingSystemVersion
                0, // MajorImageVersion
                0, // MinorImageVersion
                5, // MajorSubsystemVersion
                0, // MinorSubsystemVersion
                0, // Win32VersionValue
                2 * ALIGN, // SizeOfImage
                FILE_ALIGN, // SizeOfHeaders
                0, // checksum
                IMAGE_SUBSYSTEM_WINDOWS_CUI, // subsytem
                0, // DllCharacteristics
                0, // SizeOfStackReserve
                0, // SizeOfStackCommit
                0, // SizeOfHeapReserve
                0, // SizeOfHeapCommit
                0, // LoaderFlags
                IMAGE_DIRECTORY_ENTRY_IMPORT + 1, // number of data directories
                {
                    { }, // exports
                    { offsetof(A, importKernel32) - FILE_ALIGN + ALIGN }, // imports
                }
            }
        },
        { // section
            { 0 }, // name
            { ALIGN }, // Misc.VirtualSize
            ALIGN, // VirtualAddress
            imageSize - FILE_ALIGN, // SizeOfRawData
            FILE_ALIGN, // PointerToRawData
            0, // relocs (obj only)
            0, // lines (obj only)
            0, // relocs (obj only)
            0, // lines (obj only)
            IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ, // Characteristics
        },
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
    typedef void (*APP)(void);

    memcpy(a.app, app, functionSize);

    if (IsDebuggerPresent()) __debugbreak();

    ((APP)&app)();

    int* acall = (int*)(&a.app[((char*)&call_WriteFile - app)]);
    acall[-1] = (char*)&a.pfnWriteFile - (char*)acall;

    fwrite(&a, 1, imageSize, fopen("9.exe", "wb"));
}
