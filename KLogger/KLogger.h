//
//  KLogger.h
//  KLogger
//
//
//  Copyright © 2020 Anonymouz4. All rights reserved.
//

#ifndef KLogger_h
#define KLogger_h

int KLog(char *format, ...);

int KLogT(char* text, LOG_TYPE type);

#define LOG(str, args...) \
do { \
    KLog("[SysPlus] " str, ##args); \
} while(0)

#define LOG_PTR(str, ptr) \
do { \
    uintptr_t _p = (uintptr_t)(ptr); \
    KLog(str ": 0x%08x%08x", ((uint32_t*)&_p)[1], ((uint32_t*)&_p)[0]); \
} while(0)



#endif /* KLogger_h */
