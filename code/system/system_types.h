#ifndef SYSTEM_TYPES_H
#define SYSTEM_TYPES_H


#include <stdint.h>
#include <string.h> /* memset */
#include <stdlib.h> /* atoi */
#include <stdio.h>


#ifndef FALSE		
    #define FALSE   0
#endif

#ifndef TRUE
    #define TRUE    1
#endif

#ifndef ENABLE
#define   ENABLE    1
#endif

#ifndef DISABLE
#define   DISABLE   0
#endif
		
#define _TRUE        TRUE	
#define _FALSE	     FALSE	

#ifndef NULL
#define NULL 0
#endif

#define _LOCK	1		
#define _UNLOCK	0
#define IN
#define OUT

//#define ERRNO

#define BIT0	0x0001
#define BIT1	0x0002
#define BIT2	0x0004
#define BIT3	0x0008
#define BIT4	0x0010
#define BIT5	0x0020
#define BIT6	0x0040
#define BIT7	0x0080
#define BIT8	0x0100
#define BIT9	0x0200
#define BIT10	0x0400
#define BIT11	0x0800
#define BIT12	0x1000
#define BIT13	0x2000
#define BIT14	0x4000
#define BIT15	0x8000
#define BIT16	0x00010000
#define BIT17	0x00020000
#define BIT18	0x00040000
#define BIT19	0x00080000
#define BIT20	0x00100000
#define BIT21	0x00200000
#define BIT22	0x00400000
#define BIT23	0x00800000
#define BIT24	0x01000000
#define BIT25	0x02000000
#define BIT26	0x04000000
#define BIT27	0x08000000
#define BIT28	0x10000000
#define BIT29	0x20000000
#define BIT30	0x40000000
#define BIT31	0x80000000
#define BIT_ALL 0xFFFFFFFF

#define BIT_(__n)       (1<<(__n))

#ifndef BIT
#define BIT(n)       (1U<<(n))
#endif

#define s8        int8_t
#define u8        uint8_t
#define s16       int16_t
#define u16       uint16_t
#define s32       int32_t
#define u32       uint32_t
#define s64       int64_t
#define u64       uint64_t

typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned int   UINT;
typedef void *PVOID;




#ifndef uint8
#define uint8	unsigned char
#endif
#ifndef uint16
#define uint16	unsigned short
#endif

#ifndef uint32
#define uint32	unsigned int
#endif

#ifndef bool
#define bool	unsigned int
#define true	1
#define false	0
#endif



typedef struct { volatile int counter; } atomic_t;

typedef struct list_head _list;


#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions                 */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions                */
#define     __IO    volatile             /*!< Defines 'read / write' permissions              */


#define HAL_READ32(base, addr)				(*((volatile u32*)(base + addr)))    
#define HAL_WRITE32(base, addr, value32)	((*((volatile u32*)(base + addr))) = (value32))
#define HAL_READ16(base, addr)				(*((volatile u16*)(base + addr)))        
#define HAL_WRITE16(base, addr, value)		((*((volatile u16*)(base + addr))) = (value)) 
#define HAL_READ8(base, addr)				(*((volatile u8*)(base + addr)))            
#define HAL_WRITE8(base, addr, value)		((*((volatile u8*)(base + addr))) = value)


#define BREAK_UINT32( var, ByteNum ) \
    (uint8_t)((uint32_t)(((var) >>((ByteNum) * 8)) & 0x00FF))

#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3) \
    ((uint32_t)((uint32_t)((Byte0) & 0x00FF) \
        + ((uint32_t)((Byte1) & 0x00FF) << 8) \
            + ((uint32_t)((Byte2) & 0x00FF) << 16) \
                + ((uint32_t)((Byte3) & 0x00FF) << 24)))

#define BUILD_WORD_UINT32(lowWord,HighWord) \
                 (uint32_t)(((HighWord & 0xFFFF)<<16) + (lowWord&0xFFFF))
  
#define HI_UINT32(a) ((uint16_t) (((uint32_t)(a)) >> 16))
#define LO_UINT32(a) ((uint16_t) ((uint32_t)(a)))

#define BUILD_UINT16(loByte, hiByte) \
    ((uint16_t)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

#define HI_UINT16(a) (((uint16_t)(a) >> 8) & 0xFF)
#define LO_UINT16(a) ((uint16_t)(a) & 0xFF)

#define BUILD_UINT8(loByte, hiByte ) \
    ((uint8_t)(((loByte) & 0x0F) + (((hiByte) & 0x0F) << 4)))

#define HI_UINT8(a) (((uint8_t)(a) >> 4) & 0x0F)
#define LO_UINT8(a) (uint8_t)((a) & 0x0F)

#define S9070_VALID_KEY 		0x02039070


typedef struct system_entry_ {
    void (*main_entry) (void);
	void (*ps_entry) (void);
    u32 valid_key;
}system_entry_t;


typedef struct system_time_
{
    u16 year; //from 1900
    u8 month; //1-12   
    u8 day;   //1-31
    u8 hour;  //0~23
    u8 min;   //0~59
    u8 sec;   //0~59
    u8 week;  //0~6  sunday 0 monday 1 ...
    u32 hw_time;//unix time
}system_time_t;




















#endif
