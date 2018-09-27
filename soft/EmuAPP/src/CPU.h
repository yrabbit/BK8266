#ifndef CPU_H_INCLUDE
#define CPU_H_INCLUDE

#include <stdint.h>

// #define DEBUG_PRINT( X) printf X
#define DEBUG_PRINT( X)

typedef uint_fast32_t TCPU_Arg;
typedef uint_fast32_t TCPU_Psw;

#define CPU_FLAG_KEY_VECTOR_274 0x00000001UL
#define CPU_FLAG_KEY_VECTOR_60  0x00000002UL

typedef struct
{
    uint16_t r [8];
    TCPU_Psw psw;
    uint32_t Flags;
    uint32_t Time;

} TCPU_State;

typedef struct
{
    TCPU_State CPU_State;

    struct
    {
	    uint32_t PrevT;
    	uint8_t  T;
	    uint8_t  Div;

	} Timer;

    struct
    {
        uint16_t WrReg177714;
        uint16_t WrReg177716;

    } SysRegs;

} TDevice_Data;

extern TDevice_Data Device_Data;

void CPU_Init           (void);
void CPU_RunInstruction (void);
void CPU_Stop           (void);
void CPU_TimerRun       (void);

#define MEM8  ((uint8_t  *) 0x3FFE8000)
#define MEM16 ((uint16_t *) 0x3FFE8000)
#define MEM32 ((uint32_t *) 0x3FFE8000)

#endif
