#pragma once


#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1
#include <new>
#include <windows.h>
#include <strsafe.h>
#include <objbase.h>
#pragma warning(push)
#pragma warning(disable : 4201)
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#pragma warning(pop)
#include <setupapi.h>  
#include <initguid.h>  // Put this in to get rid of linker errors.  
#include <devpkey.h>  // Property keys defined here are now defined inline.   
#include <functiondiscoverykeys.h>