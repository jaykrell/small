// This takes the learnings of 10.cpp and produces
// printing hello in 280 bytes.

#include <stdio.h>
#include <windows.h>
#include <assert.h>

#pragma pack(1)

#if 1
extern "C"
{
extern char app[];
extern char app_end[];
extern int call_puts[];
extern int lea_hello[];
}

#else

/* linking app breaks debugging so here it is:

app:
  0000000140001000: 48 8D 0D 06 00 00  lea         rcx,[call_puts]
                    00
lea_hello:
  0000000140001007: FF 25 AB 21 00 00  jmp         qword ptr [__imp_puts]
call_puts:
  000000014000100D: 68
  000000014000100E: 69
  000000014000100F: 00
*/
struct App
{
    char a[3];
    int rhi;
    char c[2];
    int rputs;
    char hi[3];
};
extern App app;
#define app_end (1 + (char*)&app)

char hello[] = "hello";

App app =
{
    { 0x48, 0x8d, 0xd },
    &app.right[1] 
    { 0xff, 0x25 },
};  
// unfinished
#endif

int main()
{
    struct Imports
    {
        // These need not be all together.
        IMAGE_THUNK_DATA64 iat_puts;
        IMAGE_THUNK_DATA64 iat_0;
        IMAGE_IMPORT_DESCRIPTOR importMsvcrt;
        IMAGE_IMPORT_DESCRIPTOR import0;
    };

    union A
    {
        struct
        {
            USHORT DosMagic;
            char pad1[14];
            union
            {
                struct
                {
                    IMAGE_NT_HEADERS64 nt;
                };
                struct
                {
                    char pad2[offsetof(IMAGE_NT_HEADERS64, OptionalHeader.DataDirectory[2])];
                    IMAGE_SECTION_HEADER section;
                    Imports imports;
                };
                struct
                {
                    char pad3[offsetof(IMAGE_NT_HEADERS64, OptionalHeader.MajorLinkerVersion)];
                    char code[13];
                };
                struct
                {
                    // This is tricky. puts is preceded by an arbitrary 2 byte
                    // hint, which overlaps NumberOfSections, which precedes TimeDateStamp.
                    char pad4[offsetof(IMAGE_NT_HEADERS64, FileHeader.TimeDateStamp)];
                    char string_puts[sizeof("puts")];
                    char string_msvcrt[sizeof("msvcrt")];
                };
                struct
                {
                    char pad5[offsetof(IMAGE_NT_HEADERS64, FileHeader.NumberOfSections)];
                    char hint_puts[sizeof("\0\0puts")];
                };
            };
        };
        IMAGE_DOS_HEADER dos;
    };

    int functionSize = app_end - app;
    IMAGE_DOS_HEADER dos = {IMAGE_DOS_SIGNATURE};
    dos.e_lfanew = 16;

    A a =
    {
        IMAGE_DOS_SIGNATURE,
        {0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,
         0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC}, // 14 unused bytes
        {
            IMAGE_NT_SIGNATURE, // 4 bytes
            {
                IMAGE_FILE_MACHINE_AMD64, // 2 bytes
                1, // number of sections  // 2 bytes
                0, // time                  // 4 bytes
                0, // symbols (for obj files) // 4 bytes
                0, // symbols (for obj files) // 4 bytes
                offsetof(IMAGE_OPTIONAL_HEADER64, DataDirectory[2]), // 2 bytes, must be 8 aligned
                IMAGE_FILE_EXECUTABLE_IMAGE, // value=2 not an obj, required
            },
            {
                IMAGE_NT_OPTIONAL_HDR64_MAGIC,
                0, // MajorLinkerVersion
                0, // MinorLinkerVersion
                0, // SizeOfCode
                0, // SizeOfInitializedData
                0, // SizeOfUninitializedData
                0x1000 + offsetof(a, code), // AddressOfEntryPoint
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
                64 // offsetOfPe
                + 4 // pe
                + sizeof(IMAGE_FILE_HEADER)
                + offsetof(IMAGE_OPTIONAL_HEADER64, DataDirectory[2]), // SizeOfHeaders // constraints on this are not understood
                0, // checksum
                IMAGE_SUBSYSTEM_WINDOWS_CUI, // subsytem
                0, // DllCharacteristics
                0, // SizeOfStackReserve
                0, // SizeOfStackCommit
                0, // SizeOfHeapReserve
                0, // SizeOfHeapCommit
                0, // LoaderFlags
                2, // number of data directories
                {
                    { }, // exports 8 unused bytes
                    { 0x1000 + offsetof(A, imports.importMsvcrt) }, // imports
                }
            }
        }
    };

    a.dos.e_lfanew = dos.e_lfanew;

    // The largest "reusable" part of the headers
    // is DataDirectory[2-15].
    // Let's assume we are using that and it is large enough.
    // Not everything here needs to be contiguous. If we run
    // out of room, we can try to move some elsewhere.

    IMAGE_SECTION_HEADER section =
    {
        { 0 }, // name
        { 0x1000 }, // Misc.VirtualSize
        0x1000, // VirtualAddress
        0x100, // SizeOfRawData constraints on this are not understood (helps to find IAT?)
        1, // PointerToRawData // presumably rounded down to 0
        0, // relocs (obj only)
        0, // lines (obj only)
        0, // relocs (obj only)
        0, // lines (obj only)
        IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ, // Characteristics
    };

    a.section = section;

    Imports imports =
    {
        { 0x1000 + offsetof(A, hint_puts) },            // iat_puts = rva(hint_puts)
        { 0 }, // iat_0                                 // iat terminal 0
        {                                               // importMsvcrt
            { 0x1000 + offsetof(A, imports.iat_puts) }, // rva of msvcrt IAT, starting with puts
            0, // time                                  // free bytes
            0, // forwarder                             // free bytes?
            { 0x1000 + offsetof(A, string_msvcrt) },    // rva("msvcrt")
            { 0x1000 + offsetof(A, imports.iat_puts) }  // rva of msvcrt INT, same as IAT
        },
    };

    a.imports = imports;

    // It just fits. move code elsewhere.
    //assert(sizeof(imports) + functionSize <= 14 * sizeof(IMAGE_DATA_DIRECTORY));
    assert(sizeof(imports) <= 14 * sizeof(IMAGE_DATA_DIRECTORY));

    strcpy(a.string_puts, "puts");
    strcpy(a.string_msvcrt, "msvcrt");

    // The code is 13 bytes, without the strong.
    // There are 14 at MajorLinkerVersion.
    // string easily fits in section name (8 bytes)

    // Copy the app code from writer to headers.
    char* a_app = a.code;
    memcpy(a_app, app, 13);

    // Get call (or jmp) to puts in the app in headers.
    int* a_call = (int*)(&a_app[((char*)&call_puts - app)]);

    // Set offset from call/jmp to iat puts in payload.
    a_call[-1] = (char*)&a.imports.iat_puts - (char*)a_call;

    // And for the string.
    int* a_hello = (int*)(&a_app[((char*)&lea_hello - app)]);
    a_hello[-1] = ((char*)&a.section.Name - (char*)a_hello);

    strcpy((char*)a.section.Name, "hello");

    FILE* file = fopen("13.exe", "wb");

    fwrite(&a, 1, sizeof(a), file);

    fclose(file);

    system("dir 13.exe");
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
