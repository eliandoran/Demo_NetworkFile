#ifndef LOG_H_DEFINED
#define LOG_H_DEFINED

#define LOG_BASE(level, ...) (printf("%s: ", level)); (printf(__VA_ARGS__))
#define LOG_ERROR(...) LOG_BASE("ERROR", __VA_ARGS__)
#define LOG_ACCESS(...) LOG_BASE("ACCESS", __VA_ARGS__)

#ifndef LOG_QUIET
    #define LOG(...) LOG_BASE("INFO", __VA_ARGS__)
#else
    #define LOG(...) ;
#endif

#endif