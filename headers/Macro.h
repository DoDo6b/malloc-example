#ifndef MACRO_H
#define MACRO_H


#ifndef NDEBUG
    #define IF_DBG(...) __VA_ARGS__
#else
    #define IF_DBG(...)
#endif

#define OFF if(0)


#endif