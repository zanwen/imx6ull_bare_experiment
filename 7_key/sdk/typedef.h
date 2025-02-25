#ifndef __TYPEDEF_H__
#define __TYPEDEF_H__

#define __I  volatile
#define __O  volatile
#define __IO volatile

#define BIT_IS_SET(REG, BIT_INDEX)   ((REG) & (1 << (BIT_INDEX)))
#define BIT_IS_NOSET(REG, BIT_INDEX) ((REG) & (1 << (BIT_INDEX)) == 0)

#define BIT_CLEAR(REG, BIT_INDEX) (REG) &= ~(1 << (BIT_INDEX))
#define BIT_SET(REG, BIT_INDEX)   (REG) |= (1 << (BIT_INDEX))
#define BITS_SET(REG, VALUE)      (REG) |= (VALUE)

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;

#endif /* __TYPEDEF_H__ */
