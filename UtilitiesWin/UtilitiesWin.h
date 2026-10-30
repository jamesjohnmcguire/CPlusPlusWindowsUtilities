#pragma once

#include <Windows.h>

///////////////////////////////////////////////////////////////////////
// Windows and COM definitions shared by the existing projects.
///////////////////////////////////////////////////////////////////////
#define IN
#define OUT
#define INOUT
#define RETVAL DWORD

#define DllExport __declspec(dllexport)

constexpr int GENERAL_ERROR = -1;
constexpr int CONNECTION_ERROR = -2;

#define E_GENERAL_ERROR		MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x200)
#define E_CONNECTION_ERROR	MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x201)
#define E_USER_NOT_FOUND	MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x202)
#define W_NEW_CERTIFICATE	MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_ITF, 0x203)
#define E_USER_CANCEL		MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x204)
