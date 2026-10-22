#ifndef ENGINE_API_H
#define ENGINE_API_H

#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef ENGINE_BUILD_DLL
    #define ENGINE_API __declspec(dllexport)
  #else
    #define ENGINE_API
  #endif
#else
  #define ENGINE_API __attribute__((visibility("default")))
#endif

#endif /* ENGINE_API_H */
