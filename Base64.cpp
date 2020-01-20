// Base64.cpp : Defines the entry point for the console application.
//

#include "pch.h"
#include "Base64.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

unsigned char Base64DecodeChar(
	char c);
bool ZenBase64Decode(
	const char*	sEncodedBuffer,
	PBYTE	sUnEncodedBuffer,
	DWORD*	pnLenUnEncodedBuffer);

int Base64DecodeFile(TCHAR* source, TCHAR* destination)
{
	int result = -1;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
	}
	else
	{
		HANDLE	hInputFile;
		hInputFile = CreateFile(source,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hInputFile == INVALID_HANDLE_VALUE)
		{
			_tprintf(_T("Could not open file (error %d)\n"), GetLastError());
		}
		else
		{
			HANDLE	hOutputFile;
			hOutputFile = CreateFile(destination,
				GENERIC_WRITE,
				0,
				NULL,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			if (hOutputFile == INVALID_HANDLE_VALUE)
			{
				_tprintf(_T("Could not open file (error %d)\n"), GetLastError());
			}
			else
			{
				LPVOID	pInputBuffer;
				LPVOID	pOutputBuffer;
				DWORD	dwBytesRead;

				DWORD	dwFileSize = GetFileSize(hInputFile, NULL);

				pInputBuffer = malloc(dwFileSize + 1);
				pOutputBuffer = malloc(dwFileSize + 1);

				int returnCode = ReadFile(hInputFile,
					pInputBuffer,
					dwFileSize,
					&dwBytesRead,
					NULL);

				if (returnCode != 0)
				{
					returnCode = ZenBase64Decode(
						(char*)pInputBuffer,
						(PBYTE)pOutputBuffer,
						&dwBytesRead);

					if (returnCode == TRUE)
					{
						returnCode = WriteFile(hOutputFile,
							pOutputBuffer,
							dwBytesRead,
							&dwBytesRead,
							NULL);
						if (returnCode == TRUE)
						{
							result = 1;
						}
					}
				}
			}
		}
	}

	return result;
}

unsigned char Base64DecodeChar(
	char c)
{
	if (c >= 'A' && c <= 'Z')
	{
		return c - 'A';
	}
	if (c >= 'a' && c <= 'z')
	{
		return c - 'a' + 26;
	}
	if (c >= '0' && c <= '9')
	{
		return c - '0' + 52;
	}
	if (c == '+')
	{
		return 62;
	}

	return 63;
}

bool ZenBase64Decode(
	const char*	sEncodedBuffer,
	PBYTE	sUnEncodedBuffer,
	DWORD*	pnLenUnEncodedBuffer)
{
	bool	bRet				= false;
	int		nLenEncodedBuffer	= 0;
	int		nIndexOutBuffer		= 0;

	if ((sEncodedBuffer != NULL) && (sEncodedBuffer != NULL))
	{
		//nLenEncodedBuffer = (int)_tcslen( sEncodedBuffer );
		nLenEncodedBuffer = (int)strlen( sEncodedBuffer );

		for (int i=0;i<nLenEncodedBuffer;i+=4)
		{
			char c1='A',c2='A',c3='A',c4='A';
			c1 = sEncodedBuffer[i];
			if (i+1<nLenEncodedBuffer)
				c2 = sEncodedBuffer[i+1];

			if (i+2<nLenEncodedBuffer)
				c3 = sEncodedBuffer[i+2];

			if (i+3<nLenEncodedBuffer)
				c4 = sEncodedBuffer[i+3];

			unsigned char by1=0,by2=0,by3=0,by4=0;
			by1 = Base64DecodeChar(c1);
			by2 = Base64DecodeChar(c2);
			by3 = Base64DecodeChar(c3);
			by4 = Base64DecodeChar(c4);

			sUnEncodedBuffer[nIndexOutBuffer] = (by1<<2)|(by2>>4);
			nIndexOutBuffer++;

			if (c3 != '=')
			{
				sUnEncodedBuffer[nIndexOutBuffer] = ((by2&0xf)<<4)|(by3>>2);
				nIndexOutBuffer++;
			}
			if (c4 != '=')
			{
				sUnEncodedBuffer[nIndexOutBuffer] = ((by3&0x3)<<6)|by4;
				nIndexOutBuffer++;
			}
		}

		sUnEncodedBuffer[nIndexOutBuffer]	= '\0';
		*pnLenUnEncodedBuffer = nIndexOutBuffer;
		bRet	= true;
	}
	return bRet;
}
