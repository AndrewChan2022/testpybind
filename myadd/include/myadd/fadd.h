#pragma once


#if defined(_WIN32)

#if defined(MYADD_EXPORTS)
#define MYADD_API_API __declspec(dllexport)
#else
#define MYADD_API_API __declspec(dllimport)
#endif

#if defined(_MSC_VER)
// Turn off warning about lack of DLL interface
#pragma warning(disable:4251)
// Turn off warning non-dll class is base for dll-interface class.
#pragma warning(disable:4275)
// Turn off warning about identifier truncation
#pragma warning(disable:4786)
#endif


#else  // _WIN32
# if __GNUC__ >= 4 && (defined(MYADD_EXPORTS))
#   define MYADD_API_API __attribute__ ((visibility("default")))
# else
#   define MYADD_API_API /* hidden by default */
# endif
#endif  // _WIN32


namespace myadd {

MYADD_API_API float fadd(float a, float b);

}

