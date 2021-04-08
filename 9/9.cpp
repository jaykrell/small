// If filealign rounds down to, then the headers are mapped a second time.
// This uses that. Otherwise headers cannot be reused.
// Therefore some offsets which could be offsetof(A, x) are 0x1000 + offsetof(A, x)
#include <stdio.h>
#include <windows.h>
#include <stddef.h>

typedef struct _IMAGE_OPTIONAL_HEADER64_2 {
    WORD        Magic;
    BYTE        MajorLinkerVersion;
    BYTE        MinorLinkerVersion;
    DWORD       SizeOfCode;
    DWORD       SizeOfInitializedData;
    DWORD       SizeOfUninitializedData;
    DWORD       AddressOfEntryPoint;
    DWORD       BaseOfCode;
    ULONGLONG   ImageBase;
    DWORD       SectionAlignment;
    DWORD       FileAlignment;
    WORD        MajorOperatingSystemVersion;
    WORD        MinorOperatingSystemVersion;
    WORD        MajorImageVersion;
    WORD        MinorImageVersion;
    WORD        MajorSubsystemVersion;
    WORD        MinorSubsystemVersion;
    DWORD       Win32VersionValue;
    DWORD       SizeOfImage;
    DWORD       SizeOfHeaders;
    DWORD       CheckSum;
    WORD        Subsystem;
    WORD        DllCharacteristics;
    ULONGLONG   SizeOfStackReserve;
    ULONGLONG   SizeOfStackCommit;
    ULONGLONG   SizeOfHeapReserve;
    ULONGLONG   SizeOfHeapCommit;
    DWORD       LoaderFlags;
    DWORD       NumberOfRvaAndSizes;
//  IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
    IMAGE_DATA_DIRECTORY DataDirectory[3]; // why not 2?
} IMAGE_OPTIONAL_HEADER64_2, *PIMAGE_OPTIONAL_HEADER64_2;

typedef struct _IMAGE_NT_HEADERS64_2 {
    DWORD Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER64_2 OptionalHeader;
} IMAGE_NT_HEADERS64_2, *PIMAGE_NT_HEADERS64_2;

extern "C"
{
extern char app[1];
extern char app_end[1];
extern int call_WriteFile[];
}

#define FILE_ALIGN 0x1000 /* There is no section data anyway, just headers. */
#define ALIGN      0x1000

#pragma pack(1)

struct A
{
    // Overlap import data with dos header, it is a pretty good fit,
    USHORT dosMagic;
    USHORT pad3[1];
    IMAGE_THUNK_DATA64 pfnWriteFile;
    IMAGE_THUNK_DATA64 pfn0;
    IMAGE_IMPORT_DESCRIPTOR importKernel32;
    IMAGE_IMPORT_DESCRIPTOR import0;
    ULONG dosLfaNew;

    IMAGE_NT_HEADERS64_2 nt;
    IMAGE_SECTION_HEADER section;
    char unexplained[15];
    char app[0x1000]; // over allocation
};

int main()
{
    int functionSize = app_end - app;
    int imageSize = offsetof(A, app) + functionSize;
    A a =
    {
        IMAGE_DOS_SIGNATURE,
        {0},
        //{ 0x1000 + offsetof(A, strWriteFile) },
        { 0x1000 + offsetof(A, nt.OptionalHeader.MajorLinkerVersion) },
        { 0 }, // pfn0
        // importKernel
        {
            0x1000 + offsetof(A, pfnWriteFile),
            0, // time
            0, // forwarder
            //0x1000 + offsetof(A, strKernel32),
            //0x1000 + offsetof(A, nt.OptionalHeader.MinorOperatingSystemVersion),
            0x1000 + offsetof(A, nt.FileHeader.TimeDateStamp),
            0x1000 + offsetof(A, pfnWriteFile)
        },
        { }, // import0
        sizeof(IMAGE_DOS_HEADER),

        { // nt
            IMAGE_NT_SIGNATURE,
            {
                IMAGE_FILE_MACHINE_AMD64,
                1, // number of sections
                0, // time
                0, // symbols (for obj files)
                0, // symbols (for obj files)
                sizeof(IMAGE_OPTIONAL_HEADER64_2),
                IMAGE_FILE_EXECUTABLE_IMAGE // not an obj
            },
            {
                IMAGE_NT_OPTIONAL_HDR64_MAGIC,
                -1, // MajorLinkerVersion // \0\0WriteFile\0 fits here
                -1, // MinorLinkerVersion
                -1, // SizeOfCode
                -1, // SizeOfInitializedData
                -1, // SizeOfUninitializedData
                0x1000 + offsetof(A, app) - FILE_ALIGN + ALIGN, // AddressOfEntryPoint
                0, // BaseOfCode
                0x10000, // ImageBase
                ALIGN,
                FILE_ALIGN,
                5, // MajorOperatingSystemVersion
                -1, // MinorOperatingSystemVersion 
                -1, // MajorImageVersion
                -1, // MinorImageVersion
                5, // MajorSubsystemVersion
                0, // MinorSubsystemVersion
                -1, // Win32VersionValue
                3 * ALIGN, // SizeOfImage
                0x100, // SizeOfHeaders // constraints on this are not understood
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
                    { 0x1000 + offsetof(A, importKernel32) }, // imports
                }
            }
        },
        { // section
            { 0 }, // name
            { 2 * ALIGN }, // Misc.VirtualSize
            ALIGN, // VirtualAddress
            0x20, // SizeOfRawData constraints on this are not understood
            1, // PointerToRawData // presumably rounded down to 0
            0, // relocs (obj only)
            0, // lines (obj only)
            0, // relocs (obj only)
            0, // lines (obj only)
            IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ, // Characteristics
        },
        {}, // unexplained
        { } // app
    };
    typedef void (*APP)(void);

    memcpy(&a.nt.OptionalHeader.MajorLinkerVersion, "\0\0WriteFile", sizeof("\0\0WriteFile"));
    //memcpy(&a.nt.OptionalHeader.MinorOperatingSystemVersion, "kernel32", sizeof("kernel32"));
    memcpy(&a.nt.FileHeader.TimeDateStamp, "kernel32", sizeof("kernel32"));

    memcpy(a.app, app, functionSize);
    //a.app[0] = 0xcc;

    if (IsDebuggerPresent()) __debugbreak();

    ((APP)&app)();

    int* acall = (int*)(&a.app[((char*)&call_WriteFile - app)]);
    acall[-1] = ((char*)&a.pfnWriteFile - (char*)acall) - FILE_ALIGN + ALIGN;

    fwrite(&a, 1, imageSize, fopen("10.exe", "wb"));
}
