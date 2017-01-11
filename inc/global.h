#ifndef GLOBAL_H
#define GLOBAL_H

// Debug control
#define DEBUG_MODE_ENABLED

#ifdef DEBUG_MODE_ENABLED
//#define DEBUG_MODE_ENABLED_CPU
#endif

// Endian
#define HOST_BIG_ENDIAN			0x01
#define HOST_LITTLE_ENDIAN		0x02

// Unix/Linux detect endian
#ifdef __BYTE_ORDER__
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define HOST_ENDIAN HOST_BIG_ENDIAN
    #elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define HOST_ENDIAN HOST_LITTLE_ENDIAN
    #else
        #error "Endian not supported. Only support for big and little endian."
    #endif
#else

    // Windows Defines Method for detecting Endian
    #ifdef _M_IX86
        #define HOST_ENDIAN HOST_LITTLE_ENDIAN
    #else
        #ifdef _M_X86
            #define HOST_ENDIAN HOST_LITTLE_ENDIAN
        #else
            #ifdef _M_IA64
                #define HOST_ENDIAN HOST_LITTLE_ENDIAN
            #else
                #ifdef _M_ARM
                    #define HOST_ENDIAN HOST_LITTLE_ENDIAN
                #else
                    #ifdef _M_PPC
                        #define HOST_ENDIAN HOST_BIG_ENDIAN
                    #else
                        #error "Endian not known."
                    #endif
                #endif
            #endif
        #endif
    #endif
#endif

typedef unsigned char UINT8;
typedef signed char SINT8;
typedef unsigned short int UINT16;
typedef signed short int SINT16;
typedef unsigned int UINT32;
typedef signed int SINT32;
typedef UINT8 BYTE;
typedef UINT16 ADDRESS;

typedef union {
    UINT16 w;
    struct {
        #if HOST_ENDIAN == HOST_BIG_ENDIAN
            UINT8 h;
            UINT8 l;
        #elif HOST_ENDIAN == HOST_LITTLE_ENDIAN
            UINT8 l;
            UINT8 h;
        #endif
    } b;
} WORD_16;

#endif // GLOBAL_H
