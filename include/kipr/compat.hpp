#ifndef _AURORA_COMPAT_HPP_
#define _AURORA_COMPAT_HPP_

#ifdef _MSC_VER
#ifdef BUILD_AURORA_DLL
#define AURORA_DLL_EXPORT __declspec(dllexport)
#else
#define AURORA_DLL_EXPORT __declspec(dllimport)
#endif
#else
#define AURORA_DLL_EXPORT
#endif

#endif