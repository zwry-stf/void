#pragma once


#ifndef v_always_inline
#if defined(_MSC_VER)
#define v_always_inline __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define v_always_inline inline __attribute__((always_inline))
#else
#define v_always_inline inline
#endif
#endif