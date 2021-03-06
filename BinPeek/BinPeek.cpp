	// BinPeek.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main(int argc, char *argv[])
{
	HANDLE hFile;
	HANDLE hFileMapping;
	LPVOID lpFileBase;
	BOOL b32Bit = false;

	//Check for arg
	if (argc < 2)
	{
		printf("Usage: binpeek <file>\n");
		return 0;
	}

	//Open the file.
	hFile = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Unable to open file.\n");
		return 0;
	}
	
	//Map it and investigate
	hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);	
	lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

	//Do some simple checks to make sure it's the PE format
	//Check if at least the size of the DOS header
	DWORD dwFileSize;
	GetFileSize(hFile, &dwFileSize);
	if (dwFileSize < sizeof(IMAGE_DOS_HEADER))
	{
		printf("%s --> Not an EXE or DLL\n", argv[1]);
		return 0;
	}

	//Check the first 2 bytes for "MZ"
	BYTE *bArray = (BYTE *)lpFileBase;
	if (!((bArray[0] == 0x4D) && (bArray[1] == 0x5A)))
	{
		printf("%s --> Not an EXE or DLL\n", argv[1]);
		return 0;
	}

	//Check to make sure it's at least the size of the dos plus nt headers

	//Get the parts of the file
	PIMAGE_DOS_HEADER pidh = (PIMAGE_DOS_HEADER)lpFileBase;
	PIMAGE_NT_HEADERS pinh = (PIMAGE_NT_HEADERS)((BYTE*)pidh + pidh->e_lfanew);

	//Check for 32-bit or 64-bit
	if (pinh->FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
	{
		b32Bit = true;
	}

	//Headers are different for 32-bit and 64-bit
	BOOL bUnmanaged = false;
	if (b32Bit)
	{
		PIMAGE_OPTIONAL_HEADER pioh = (PIMAGE_OPTIONAL_HEADER)&pinh->OptionalHeader;
		bUnmanaged = pioh->DataDirectory[14].VirtualAddress == 0;
	}
	else
	{
		PIMAGE_OPTIONAL_HEADER64 pioh = (PIMAGE_OPTIONAL_HEADER64)&pinh->OptionalHeader;
		bUnmanaged = pioh->DataDirectory[14].VirtualAddress == 0;
	}

	//If the COM descriptor entry is 0, it's unmanaged
	if (bUnmanaged)
	{
		printf("%s --> Unmanaged\n", argv[1]);
	}
	else
	{
		printf("%s --> Managed\n", argv[1]);
	}
	return 0;
}
