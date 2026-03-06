#pragma once
/**
 * @file export.h
 * @brief Макрос экспорта символов для сборки DLL/SO
 */
#if defined(_WIN32)
	#if defined(ESM_BUILD_DLL)
		#define ESM_API __declspec(dllexport)
	#elif defined(ESM_USE_DLL)
		#define ESM_API __declspec(dllimport)
	#else
		#define ESM_API
	#endif
#else
	#define ESM_API __attribute__((visibility("default")))
#endif