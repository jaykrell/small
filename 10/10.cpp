/* This program is a small testbed for PE limits.

It is believed that generally LoadLibrary is easier to placate
than CreateProcess.

Through editing this program it can be found the following limits.

 - file size must be >= PeOffset + sizeof(IMAGE_NT_HEADERS) //native
   In particular. SizeOfOptionalHeader and NumberOfRvaAndSizes do not reduce that limit, unfortunately.
   You must provide the 16 data directories.

 - That would seem like 64 (dos header) + sizeof(IMAGE_NT_HEADERS64) (328)
   But dos header and PE can overlap

 - How much can they overlap?
   PE header must be 4 aligned (try values 65, 66, etc.)
   Setting PE offset to 0 obviously does not let signatures work.
   Setting PE offset to 4 or 8 seems to establish "impossible"
   or "large" values, i.e. because the PE header itself overlaps
   e_lfanew, and must contain certain values there.

 - PeOffset == 16 works. e_lfanew ends up as BaseOfCode which
   is basically meaningless.

 - The smallest PE acceptable by LoadLibrary is therefore 280 bytes.
   At least on a 64bit kernel. 32bit might accept smaller.

 - This size also is viable with CreateProcess, i.e. returning 42.

The challenge will then be, either how much functionality to fit in 280
bytes, or what expansion is worth what functionality.

A 264 byte image is also likely possible on a 32bit system.

In an image with no imports, there are about 176 free contiguous
bytes in the data directories. Or 160 if you have imports. Plus
a few available bytes scattered around.

This 280 LoadLibrary'able PE is probably the limit in terms
of lowest possible number of set bits.
*/
#include <stdio.h>
#include <windows.h>

int main()
{
    FILE* file;

    printf("establish baseline\n");

    IMAGE_DOS_HEADER dos = {IMAGE_DOS_SIGNATURE};
    dos.e_lfanew = 16;

    IMAGE_NT_HEADERS64 nt =
    {
        IMAGE_NT_SIGNATURE, // 4
        {
            IMAGE_FILE_MACHINE_AMD64, // 4
            0, // number of sections  // 2
            0, // time                  // 4
            0, // symbols (for obj files) // 4
            0, // symbols (for obj files) // 4
            //sizeof(IMAGE_OPTIONAL_HEADER64) - sizeof(IMAGE_SECTION_HEADER),
            0, // wow
            2 // not an obj
        },
        {
            IMAGE_NT_OPTIONAL_HDR64_MAGIC,
            0, // MajorLinkerVersion
            0, // MinorLinkerVersion
            0, // SizeOfCode
            0, // SizeOfInitializedData
            0, // SizeOfUninitializedData
            0, // AddressOfEntryPoint
            0, // BaseOfCode
            0, // ImageBase
            1, // SectionAlignment
            1, // FileAlignment
            0, // MajorOperatingSystemVersion
            0, // MinorOperatingSystemVersion 
            0, // MajorImageVersion
            0, // MinorImageVersion
            0, // MajorSubsystemVersion
            0, // MinorSubsystemVersion
            0, // Win32VersionValue
            0x100, // SizeOfImage in virtual address space 0x98 works but let's set fewer bits
            0, // SizeOfHeaders // constraints on this are not understood
            0, // checksum
            0, // subsytem
            0, // DllCharacteristics
            0, // SizeOfStackReserve
            0, // SizeOfStackCommit
            0, // SizeOfHeapReserve
            0, // SizeOfHeapCommit
            0, // LoaderFlags
            0, // number of data directories
        }
    };
    /*
    IMAGE_SECTION_HEADER section =
    { // section
        { 0 }, // name
        { 0x1000 }, // Misc.VirtualSize
        0x1000, // VirtualAddress
        0x200, // SizeOfRawData constraints on this are not understood
        0x200, // PointerToRawData // presumably rounded down to 0
        0, // relocs (obj only)
        0, // lines (obj only)
        0, // relocs (obj only)
        0, // lines (obj only)
        0, // Characteristics
    };
    */
    //((IMAGE_SECTION_HEADER*)(&nt + 1))[-1] = section;

    file = fopen("1.dll", "wb");

    fseek(file, 0, SEEK_SET);
    fwrite(&dos, 1, sizeof(dos), file);

    fseek(file, dos.e_lfanew, SEEK_SET);
    fwrite(&nt, 1, sizeof(nt), file);

    fseek(file, 60, SEEK_SET);
    fwrite(&dos.e_lfanew, 1, sizeof(dos.e_lfanew), file);

    //fwrite(&section, 1, sizeof(section), file);

    //char data[0x200] = { };
    //fseek(file, 0x200, SEEK_SET);
    //fwrite(&data, 1, sizeof(data), file);

    fclose(file);

    system("dir 1.dll");
    HMODULE h = LoadLibrary("1.dll");
    int err = h ? 0 : GetLastError();
    printf("LoadLibrary:%p err:%d\n", h, err);
    if (h)
    {
        printf("baseline established\n");
        FreeLibrary(h);
    }
    else
    {
        printf("baseline failed\n");
        return err;
    }
}
