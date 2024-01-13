#include "ets.h"

#include "gpio_lib.h"

#include "tv.h"
#include "ps2.h"
//#include "keymap.h"
//#include "tape.h"
#include "timer0.h"
#include "ui.h"
#include "menu.h"
#include "ffs.h"
//#include "zkg.h"
#include "reboot.h"
//#include "help.h"
#include "str.h"
#include "board.h"
#include "CPU.h"
#include "Key.h"

union
{
    uint32_t U32 [64];
    uint16_t U16 [64 * 2];
    uint8_t  U8  [64 * 4];

} TapeBuf;

#define CPU_CHECK_ARG_FAULT( Arg) {if (CPU_IS_ARG_FAULT (Arg)) goto BusFault;}

static void PrintR0(void) {
	ets_printf("R:%x\n", Device_Data.CPU_State.r [0]);
    Device_Data.CPU_State.r [7] = 0100006;
}

static void PrintZ(void) {
	ets_printf("Z%x\n", Device_Data.CPU_State.r [0]);
    Device_Data.CPU_State.r [7] = 0100006;
}

static void PrintStr(void) {
	uint_fast16_t adr = Device_Data.CPU_State.r [0];
	do {
		uint8_t ch = CPU_ReadMemB(adr++) & 0xFFU;
		if (!ch) {
			break;
		}
	    ets_printf("%c", ch);
	} while(1);
	ets_printf("\n");
    Device_Data.CPU_State.r [7] = 0100006;
}

static void PrintR0D(void) {
	ets_printf("%d ", Device_Data.CPU_State.r [0]);
    Device_Data.CPU_State.r [7] = 0100006;
}

static void TapeReadOp (void)
{
    uint_fast16_t ParamAdr = MEM16 [0306 >> 1];
    uint_fast16_t Addr     = 01000;
    uint_fast16_t Size     = 01000;
    int           iFile;
    uint_fast16_t Offset;
    uint_fast32_t CheckSum;
    uint_fast16_t Count;
    TCPU_Arg      Arg;

    MEM16 [0304 >> 1] = 1;

    for (Count = 0; Count < 16; Count++)
    {
        Arg = CPU_ReadMemB ((ParamAdr + 6 + Count) & 0xFFFFU);

        CPU_CHECK_ARG_FAULT (Arg);

        TapeBuf.U8 [Count] = (uint8_t) Arg;
    }

    ffs_cvt_name ((char *) &TapeBuf.U8 [0], (char *) &TapeBuf.U8 [16]);

    if (TapeBuf.U8 [16] == 0)
    {
        ui_start();
        iFile = menu_fileman (1);
        ui_stop();

        if (iFile < 0) goto BusFault;

        ets_memcpy (&TapeBuf.U8 [0], ffs_name ((uint16_t) iFile), 16);
        for (Count = 0; (Count < 16) && (TapeBuf.U8 [Count] != 0); Count++);
        for (;          (Count < 16);                              Count++) TapeBuf.U8 [Count] = ' ';
    }
    else
    {
        iFile = ffs_find ((char *) &TapeBuf.U8 [16]);
    }

    if (iFile < 0)
    {
//      ets_memcpy (&TapeBuf.U8 [0], "FILE NOT FOUND  ", 16);
        MEM8 [0301] = 2;
    }
    else if (fat [iFile].size < 4)
    {
        MEM8 [0301] = 2;
    }
    else
    {
        ffs_read ((uint16_t) iFile, 0, (uint8_t *) &TapeBuf.U16 [8], 4);
        Addr = TapeBuf.U16 [8];
        Size = TapeBuf.U16 [9];

        if ((uint_fast32_t) Size + 4 > fat [iFile].size)
        {
            MEM8 [0301] = 2;
        }
    }

    CPU_CHECK_ARG_FAULT (CPU_WriteMemW ((ParamAdr + 22) & 0xFFFFU, Addr));
    CPU_CHECK_ARG_FAULT (CPU_WriteMemW ((ParamAdr + 24) & 0xFFFFU, Size));

    for (Count = 0; Count < 16; Count++)
    {
        CPU_CHECK_ARG_FAULT (CPU_WriteMemB ((ParamAdr + 26 + Count) & 0xFFFFU, TapeBuf.U8 [Count]));
    }

    if (MEM8 [0301]) goto Exit;

                   Arg = CPU_ReadMemW ((ParamAdr + 24) & 0xFFFFU); CPU_CHECK_ARG_FAULT (Arg);   MEM16 [0266 >> 1] = Arg;
                   Arg = CPU_ReadMemW ((ParamAdr +  2) & 0xFFFFU); CPU_CHECK_ARG_FAULT (Arg);
    if (Arg == 0) {Arg = CPU_ReadMemW ((ParamAdr + 22) & 0xFFFFU); CPU_CHECK_ARG_FAULT (Arg);}  MEM16 [0264 >> 1] = Arg;

    Addr     = MEM16 [0264 >> 1];
    Size     = MEM16 [0266 >> 1];
    CheckSum = 0;
    Offset   = 4;

    while (Size)
    {
        uint_fast16_t TempSize;

        TempSize = Size;
        if (TempSize > sizeof (TapeBuf) / sizeof (uint8_t)) TempSize = sizeof (TapeBuf) / sizeof (uint8_t);

        ffs_read ((uint16_t) iFile, Offset, &TapeBuf.U8 [0], sizeof (TapeBuf));

        for (Count = 0; Count < TempSize; Count++)
        {
            CheckSum += (uint_fast32_t) TapeBuf.U8 [Count];
            Arg = CPU_WriteMemB (Addr, TapeBuf.U8 [Count]);
            Addr = (Addr + 1) & 0xFFFFU;
            CPU_CHECK_ARG_FAULT (Arg);
        }

        Offset += TempSize;
        Size   -= TempSize;
    }

    CheckSum = (CheckSum & 0xFFFFU) + (CheckSum >> 16);
    CheckSum = (CheckSum & 0xFFFFU) + (CheckSum >> 16);
    MEM16 [0312 >> 1] = (uint16_t) CheckSum;
    Device_Data.CPU_State.r [5] = Addr;

Exit:

    Device_Data.CPU_State.r [7] = 0116232;
    return;

BusFault:

    Device_Data.CPU_State.r [7] = MEM16 [04 >> 1];
    Device_Data.CPU_State.psw   = MEM16 [06 >> 1];
}

static void TapeWriteOp (void)
{
    uint_fast16_t ParamAdr = Device_Data.CPU_State.r [1];
    int           iFile    = -1;
    uint_fast32_t CheckSum;
    TCPU_Arg      Arg;
    uint_fast16_t Addr;
    uint_fast16_t Size;
    uint_fast16_t offs;
    uint_fast16_t Count;
    uint_fast8_t  FirstOffs;

    Arg = CPU_ReadMemW (ParamAdr + 2); CPU_CHECK_ARG_FAULT (Arg); Addr = Arg;
    Arg = CPU_ReadMemW (ParamAdr + 4); CPU_CHECK_ARG_FAULT (Arg); Size = Arg;

    for (Count = 0; Count < 16; Count++)
    {
        Arg = CPU_ReadMemB ((ParamAdr + 6 + Count) & 0xFFFFU);

        CPU_CHECK_ARG_FAULT (Arg);

        TapeBuf.U8 [Count] = (uint8_t) Arg;
    }

    ffs_cvt_name ((char *) &TapeBuf.U8 [0], (char *) &TapeBuf.U8 [0]);

    if (ffs_find ((char *) &TapeBuf.U8 [0]) >= 0) goto BusFault; // ���� ��� ����������

    // ������� ����
    iFile = ffs_create ((char *) &TapeBuf.U8 [0], TYPE_TAPE, Size + sizeof (uint32_t));

    if (iFile < 0) goto BusFault; // ���� �� ��������

    TapeBuf.U16 [0] = Addr;
    TapeBuf.U16 [1] = Size;
    FirstOffs       = 4;
    offs            = 0;
    CheckSum        = 0;

    while (Size)
    {
        uint_fast16_t RdSize;
        uint_fast16_t WrSize;
        uint8_t       *pBuf;

        pBuf   = &TapeBuf.U8 [FirstOffs];
        RdSize = sizeof (TapeBuf) / sizeof (uint8_t) - FirstOffs;
        if (RdSize > Size) RdSize = Size;
        WrSize = (RdSize + FirstOffs + 3) & ~3U;
        FirstOffs = 0;

        for (Count = 0; Count < RdSize; Count++)
        {
            Arg = CPU_ReadMemB (Addr);
            Addr = (Addr + 1) & 0xFFFFU;
            CPU_CHECK_ARG_FAULT (Arg);
            *pBuf++   = (uint8_t)       Arg;
            CheckSum += (uint_fast32_t) Arg;
        }

        ffs_writeData (iFile, offs, &TapeBuf.U8 [0], WrSize);

        offs += WrSize;
        Size -= RdSize;
    }

    CheckSum = (CheckSum & 0xFFFFU) + (CheckSum >> 16);
    CheckSum = (CheckSum & 0xFFFFU) + (CheckSum >> 16);
    MEM16 [0312 >> 1] = (uint16_t) CheckSum;

    Device_Data.CPU_State.r [7] = 0116232;
    return;

BusFault:

    if (iFile >= 0) ffs_remove (iFile);

    Device_Data.CPU_State.r [7] = MEM16 [04 >> 1];
    Device_Data.CPU_State.psw   = MEM16 [06 >> 1];
}

#undef CPU_CHECK_ARG_FAULT

void main_program(void)
{
    int_fast32_t  Time;
    uint_fast32_t T            = 0;
    uint_fast16_t CodeAndFlags;
    uint_fast16_t Key;
    uint_fast32_t LastKey      = 0xC00;
    uint_fast8_t  RunState     = 0;
    // ������ �������� �������
    ffs_init();

    // ������ �����
    tv_init  ();
    tv_start ();
    // ������ ����������
    //ps2_init ();

	// player pins (177714)
    gpio_init_output(PLAYER_CLK);
    gpio_init_output(PLAYER_IN);
    gpio_init_input(PLAYER_OUT);

    // ������ ���������
    CPU_Init ();

    Time = getCycleCount ();


    // ��������� ��������
    while (1)
    {
        uint_fast8_t  Count;

        if (Key_Flags & KEY_FLAGS_TURBO)
        {
            for (Count = 0; Count < 16; Count++)
            {
                CPU_RunInstruction ();

                if (Device_Data.CPU_State.r [7] == 0116142 && 0) // 0116076
                {
                    uint_fast16_t Cmd = Device_Data.CPU_State.r [0];

                    if      (Cmd == 3) TapeReadOp  ();
                    else if (Cmd == 2) TapeWriteOp ();
                }
            }

            Time = getCycleCount ();
            T    = Device_Data.CPU_State.Time;
        }
        else
        {
            uint_fast32_t NewT;

            for (Count = 0; Count < 16; Count++)
            {
                if ((int32_t) (Time - getCycleCount ()) > 0) break;

                CPU_RunInstruction ();

                NewT  = Device_Data.CPU_State.Time;
                Time += (uint32_t) (NewT - T) * 53;
                T     = NewT;

                if (Device_Data.CPU_State.r [7] == 0116142 && 0) // 0116076
                {
                    uint_fast16_t Cmd = Device_Data.CPU_State.r [0];

                    if      (Cmd == 3) {TapeReadOp  (); Time = getCycleCount (); T = Device_Data.CPU_State.Time;}
                    else if (Cmd == 2) {TapeWriteOp (); Time = getCycleCount (); T = Device_Data.CPU_State.Time;}
                }
                if (Device_Data.CPU_State.r [7] == 0100004) // 0100006
                {
                    uint_fast16_t Cmd = Device_Data.CPU_State.r [5];

                   if (Cmd == 0) {PrintR0(); Time = getCycleCount (); T = Device_Data.CPU_State.Time;}
                   if (Cmd == 1) {PrintZ(); Time = getCycleCount (); T = Device_Data.CPU_State.Time;}
                   if (Cmd == 2) {PrintStr(); Time = getCycleCount (); T = Device_Data.CPU_State.Time;}
                   if (Cmd == 3) {PrintR0D(); Time = getCycleCount (); T = Device_Data.CPU_State.Time;}
                }
            }

            NewT = getCycleCount ();
            if ((int32_t) (NewT - Time) > 0x10000L) Time = NewT - 0x10000L;
        }

        // ��� ���������

        switch (RunState++)
        {
            default:
            case 0:
                CPU_TimerRun ();
                RunState = 1;
                break;

            case 1:
                //ps2_periodic ();
                break;

            case 2:
                //CodeAndFlags = ps2_read ();
                if (CodeAndFlags == 0) RunState = 5;
                break;

            case 3:
                Key = Key_Translate (CodeAndFlags);
                break;

            case 4:
                //ps2_leds ((Key_Flags >> KEY_FLAGS_CAPSLOCK_POS) & 1, (Key_Flags >> KEY_FLAGS_NUMLOCK_POS) & 1, (Key_Flags >> KEY_FLAGS_TURBO_POS) & 1);

				/*
                if (Key_Flags & KEY_FLAGS_NUMLOCK) MEM16 [0177714 >> 1] = (uint16_t) (Key_Flags >> KEY_FLAGS_UP_POS);
                else                               MEM16 [0177714 >> 1] = 0;
				*/

                if (CodeAndFlags & 0x8000U)
                {
                    if (((LastKey ^ CodeAndFlags) & 0x7FF) == 0)
                    {
                        MEM16 [0177716 >> 1] |= 0100;

                        LastKey = 0xC00;
                    }
                }
                else if (Key != KEY_UNKNOWN)
                {
                    if (Key == KEY_MENU_ESC)
                    {
                        ui_start();
                        menu ();
                        ui_stop();

                        Time = getCycleCount ();
                        T    = Device_Data.CPU_State.Time;
                    }
                    else
                    {
                        LastKey  = ((uint_fast32_t) Key << 16) | CodeAndFlags;

                        RunState = 6;
                    }
                }
                break;

            case 6:

                MEM16 [0177716 >> 1] &= ~0100;

            case 5:

                if ((LastKey & 0x800) == 0)
                {
                    if ((MEM16 [0177660 >> 1] & 0200) == 0)
                    {
                        Key = (uint_fast16_t) (LastKey >> 16);

                        if      (Key == 14) Key_SetRusLat ();
                        else if (Key == 15) Key_ClrRusLat ();

                        if ((MEM16 [0177660 >> 1] & 0100) == 0)
                        {
                            if (Key & KEY_AR2_PRESSED)
                            {
                                Device_Data.CPU_State.Flags &= ~CPU_FLAG_KEY_VECTOR_60;
                                Device_Data.CPU_State.Flags |=  CPU_FLAG_KEY_VECTOR_274;
                            }
                            else
                            {
                                Device_Data.CPU_State.Flags &= ~CPU_FLAG_KEY_VECTOR_274;
                                Device_Data.CPU_State.Flags |=  CPU_FLAG_KEY_VECTOR_60;
                            }
                        }

                        MEM16 [0177660 >> 1] |= 0200;
                    }

                    LastKey |= 0x800;
                    MEM16 [0177662 >> 1] = (uint16_t) (LastKey >> 16) & 0177;
                }

                RunState = 0;

                break;
        }
    }
}
