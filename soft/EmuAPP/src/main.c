#include "ets.h"

#include "tv.h"
#include "ps2.h"
//#include "keymap.h"
//#include "tape.h"
#include "timer0.h"
//#include "ui.h"
//#include "menu.h"
#include "ffs.h"
//#include "zkg.h"
#include "reboot.h"
//#include "help.h"
#include "str.h"
#include "board.h"
#include "CPU.h"
#include "Key.h"

static void TapeOp (void)
{
    uint_fast16_t ParamAdr = Device_Data.CPU_State.r [1];
    uint_fast8_t  Cmd      = MEM8  [ ParamAdr          ];
    uint_fast16_t Addr     = MEM16 [(ParamAdr + 2) >> 1];
    uint_fast16_t Size     = MEM16 [(ParamAdr + 4) >> 1];

    static uint32_t Buf [32];

    ets_memcpy (Buf, &MEM8 [ParamAdr + 6], 16);

    Buf [4] = 0;

    if (Cmd == 3)
    {
        int  iFile = ffs_find ((char *) &Buf [0]);

        if (iFile < 0)
        {
            Addr = 0;
            MEM8 [ParamAdr + 1] = 2;
        }
        else
        {
            uint_fast16_t Offset;
            // ffs_read (uint16_t n, uint16_t offs, uint8_t *data, uint16_t size)

            ffs_read ((uint16_t) iFile, 0, (uint8_t *) &Buf [0], 4);

            if (Addr == 0) Addr = ((uint16_t *) &Buf [0]) [0];
            Size = ((uint16_t *) &Buf [0]) [1];

            MEM8  [ParamAdr + 1] = 0;
            MEM16 [0264 >> 1   ] = Addr;
            MEM16 [0266 >> 1   ] = Size;
            MEM16 [(ParamAdr + 026) >> 1] = Addr;
            MEM16 [(ParamAdr + 030) >> 1] = Size;

            Offset = 4;

            while (Size)
            {
                ffs_read ((uint16_t) iFile, Offset, (uint8_t *) &Buf [0], sizeof (Buf));

                if (Size >= sizeof (Buf)) {ets_memcpy (&MEM8 [Addr], Buf, sizeof (Buf)); Addr += sizeof (Buf); Size -= sizeof (Buf);}
                else                      {ets_memcpy (&MEM8 [Addr], Buf,         Size); Addr += Size; break;}

                Offset += sizeof (Buf);
            }

            ets_memcpy (&MEM8 [ParamAdr + 032], ffs_name ((uint16_t) iFile), 16);

            {
                int Count;

                for (Count = 0; (Count < 16) && (MEM8 [ParamAdr + 032 + Count] != 0); Count++);
                for (;          (Count < 16);                                         Count++) MEM8 [ParamAdr + 032 + Count] = ' ';
            }
        }

        Device_Data.CPU_State.r [5] = Addr;
        Device_Data.CPU_State.r [7] = MEM16 [Device_Data.CPU_State.r [6] >> 1];

        Device_Data.CPU_State.r [6] += 2;
    }
}

void main_program(void)
{
    int_fast32_t  Time;
    uint_fast32_t T   = 0;
    // Инитим файловую систему
    ffs_init();
    
    // Инитим экран
    tv_init  ();
    tv_start ();
    // Инитим клавиатуру
    ps2_init ();
    
    // Инитим процессор
    CPU_Init ();

    Time = getCycleCount ();

    
    // Запускаем эмуляцию
    while (1)
    {
        uint_fast8_t  Count;

        if (Key_Flags & KEY_FLAGS_TURBO)
        {
            for (Count = 0; Count < 16; Count++)
            {
                CPU_RunInstruction ();

                if (Device_Data.CPU_State.r [7] == 0116076) TapeOp ();
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

                if (Device_Data.CPU_State.r [7] == 0116076) TapeOp ();
            }

            NewT = getCycleCount ();
            if ((int32_t) (NewT - Time) > 0x10000L) Time = NewT - 0x10000L;
        }

        // Вся периодика

        ps2_periodic ();

        {
            static uint16_t LastKey = 0x8000U;
            uint_fast16_t CodeAndFlags = ps2_read ();

            if (CodeAndFlags)
            {
                uint_fast16_t Key = Key_Translate (CodeAndFlags);

                MEM16 [0177714 >> 1] = (uint16_t) (Key_Flags >> KEY_FLAGS_UP_POS);

                if (CodeAndFlags & 0x8000U)
                {
                    if (LastKey == (CodeAndFlags ^ 0x8000U)) MEM16 [0177716 >> 1] |= 0100;
                }
                else
                {
                    if (Key != KEY_UNKNOWN)
                    {
                        uint_fast8_t Key7 = Key & 0177;
                        if      (Key7 == 14) Key_SetNewRusLat ();
                        else if (Key7 == 15) Key_ClrNewRusLat ();

                        LastKey = CodeAndFlags;

                        MEM16 [0177716 >> 1] &= ~0100;

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
                        MEM16 [0177662 >> 1]  = (uint16_t) Key7;
                    }
                }

                ps2_leds ((Key_Flags >> KEY_FLAGS_CAPSLOCK_POS) & 1, (Key_Flags >> KEY_FLAGS_NUMLOCK_POS) & 1, (Key_Flags >> KEY_FLAGS_TURBO_POS) & 1);
            }
        }
    }
}
