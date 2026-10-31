/////////////////////////////////////////////////////////////////////////////
// Base64.cpp : Defines the entry point for the console application.
//
// Copyright © 2007 - 2026 by James John McGuire
// All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Base64.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

unsigned char Base64DecodeChar(
	char character);
bool ZenBase64Decode(
	const char*	encodedBuffer,
	PBYTE	decodedBuffer,
	DWORD*	decodedLength);

int Base64DecodeFile(TCHAR* source, TCHAR* destination)
{
	int result = -1;

	HANDLE	inputFileHandle;
	inputFileHandle = CreateFile(source,
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);

	if (inputFileHandle == INVALID_HANDLE_VALUE)
	{
		_tprintf(_T("Could not open file (error %d)\n"), GetLastError());
	}
	else
	{
		HANDLE	outputFileHandle;
		outputFileHandle = CreateFile(destination,
			GENERIC_WRITE,
			0,
			nullptr,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			nullptr);

		if (outputFileHandle == INVALID_HANDLE_VALUE)
		{
			_tprintf(_T("Could not open file (error %d)\n"), GetLastError());
		}
		else
		{
			LPVOID	inputBuffer;
			LPVOID	outputBuffer;
			DWORD	bytesRead;

			DWORD	fileSize = GetFileSize(inputFileHandle, nullptr);

			inputBuffer = malloc(fileSize + 1);
			outputBuffer = malloc(fileSize + 1);

			int returnCode = ReadFile(inputFileHandle,
				inputBuffer,
				fileSize,
				&bytesRead,
				nullptr);

			if (returnCode != 0)
			{
				returnCode = ZenBase64Decode(
					(char*)inputBuffer,
					(PBYTE)outputBuffer,
					&bytesRead);

				if (returnCode == TRUE)
				{
					returnCode = WriteFile(outputFileHandle,
						outputBuffer,
						bytesRead,
						&bytesRead,
						nullptr);
					if (returnCode == TRUE)
					{
						result = 1;
					}
				}
			}
		}
	}

	return result;
}

unsigned char Base64DecodeChar(
	char character)
{
	if (character >= 'A' && character <= 'Z')
	{
		return character - 'A';
	}
	if (character >= 'a' && character <= 'z')
	{
		return character - 'a' + 26;
	}
	if (character >= '0' && character <= '9')
	{
		return character - '0' + 52;
	}
	if (character == '+')
	{
		return 62;
	}

	return 63;
}

bool ZenBase64Decode(
	const char*	encodedBuffer,
	PBYTE	decodedBuffer,
	DWORD*	decodedLength)
{
	bool	result			= false;
	int		encodedLength	= 0;
	int		decodedIndex	= 0;

	if ((encodedBuffer != nullptr) && (encodedBuffer != nullptr))
	{
#if BASE64_TCHAR_INPUT
		encodedLength = (int)_tcslen( encodedBuffer );
#endif
		encodedLength = (int)strlen( encodedBuffer );

		for (int encodedIndex=0;encodedIndex<encodedLength;encodedIndex+=4)
		{
			char firstCharacter='A',secondCharacter='A',thirdCharacter='A',
				fourthCharacter='A';
			firstCharacter = encodedBuffer[encodedIndex];
			if (encodedIndex+1<encodedLength)
				secondCharacter = encodedBuffer[encodedIndex+1];

			if (encodedIndex+2<encodedLength)
				thirdCharacter = encodedBuffer[encodedIndex+2];

			if (encodedIndex+3<encodedLength)
				fourthCharacter = encodedBuffer[encodedIndex+3];

			unsigned char firstValue=0,secondValue=0,thirdValue=0,
				fourthValue=0;
			firstValue = Base64DecodeChar(firstCharacter);
			secondValue = Base64DecodeChar(secondCharacter);
			thirdValue = Base64DecodeChar(thirdCharacter);
			fourthValue = Base64DecodeChar(fourthCharacter);

			decodedBuffer[decodedIndex] = (firstValue<<2)|(secondValue>>4);
			decodedIndex++;

			if (thirdCharacter != '=')
			{
				decodedBuffer[decodedIndex] = ((secondValue&0xf)<<4)|(thirdValue>>2);
				decodedIndex++;
			}
			if (fourthCharacter != '=')
			{
				decodedBuffer[decodedIndex] = ((thirdValue&0x3)<<6)|fourthValue;
				decodedIndex++;
			}
		}

		decodedBuffer[decodedIndex]	= '\0';
		*decodedLength = decodedIndex;
		result	= true;
	}
	return result;
}
