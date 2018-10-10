#include "ets.h"

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

static void TapeOp (void)
{
    uint_fast16_t ParamAdr = Device_Data.CPU_State.r [1];
    uint_fast8_t  Cmd      = MEM8  [ ParamAdr          ];
    uint_fast16_t Addr     = MEM16 [(ParamAdr + 2) >> 1];
    uint_fast16_t Size     = MEM16 [(ParamAdr + 4) >> 1];

    static uint32_t Buf [64];

    ets_memcpy (Buf, &MEM8 [ParamAdr + 6], 16);

    Buf [4] = 0;

    ffs_cvt_name ((char *) &Buf [0], (char *) &Buf [0]);

    if (Cmd == 3)
    {
        int iFile;

        if (((char *) &Buf [0]) [0] == 0)
        {
            ui_start();
            iFile = menu_fileman (1);
            ui_stop();
        }
        else
        {
            iFile = ffs_find ((char *) &Buf [0]);
        }

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

            if (((uint_fast32_t) Addr + Size) > 0100000)
            {
                Addr = 0;
                MEM8 [ParamAdr + 1] = 2;
            }
            else
            {
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
        }

        Device_Data.CPU_State.r [5] = Addr;
        Device_Data.CPU_State.r [7] = MEM16 [Device_Data.CPU_State.r [6] >> 1];
        Device_Data.CPU_State.r [6] += 2;
    }
    else if (Cmd == 2) // Сохранение файла
    {
        if (((uint_fast32_t) Addr + Size) > 0100000)
        {
            MEM8 [ParamAdr + 1] = 2;
        }
        else
        {
            int iFile = ffs_find ((char *) &Buf [0]);

            if (iFile >= 0) // Файл уже существует
            {
                MEM8 [ParamAdr + 1] = 2;
            }
            else
            {
                // Создаем файл
                iFile = ffs_create ((char *) &Buf [0], TYPE_TAPE, Size + sizeof (uint32_t));

                if (iFile < 0) // Файл не создался
                {
                    MEM8 [ParamAdr + 1] = 2;
                }
                else
                {
                    uint_fast16_t offs = sizeof (Buf) - sizeof (uint32_t);

                    MEM8  [ParamAdr + 1] = 0;
                    MEM16 [0264 >> 1   ] = Addr;
                    MEM16 [0266 >> 1   ] = Size;
                    MEM16 [(ParamAdr + 026) >> 1] = Addr;
                    MEM16 [(ParamAdr + 030) >> 1] = Size;

                    ets_memcpy (&MEM8 [ParamAdr + 032], ffs_name ((uint16_t) iFile), 16);

                    ((uint16_t *) &Buf [0]) [0] = Addr;
                    ((uint16_t *) &Buf [0]) [1] = Size;

                    if (Size > sizeof (Buf) - sizeof (uint32_t))
                    {
                        ets_memcpy (&Buf [1], &MEM8 [Addr], sizeof (Buf) - sizeof (uint32_t));
                        Size -= sizeof (Buf) - sizeof (uint32_t);
                        Addr += sizeof (Buf) - sizeof (uint32_t);
                    }
                    else
                    {
                        ets_memcpy (&Buf [1], &MEM8 [Addr], Size);
                        Size = 0;
                    }
                    ffs_writeData (iFile, 0, (uint8_t *) &Buf [0], sizeof (Buf));

                    while (Size)
                    {
                        if (Size > sizeof (Buf))
                        {
                            ets_memcpy (Buf, &MEM8 [Addr], sizeof (Buf));
                            Size -= sizeof (Buf);
                            Addr += sizeof (Buf);
                        }
                        else
                        {
                            ets_memcpy (Buf, &MEM8 [Addr], Size);
                            Size = 0;
                        }

                        ffs_writeData (iFile, offs, (uint8_t *) &Buf [0], sizeof (Buf));
                        offs += sizeof (Buf);
                    }
                }
            }
        }

        Device_Data.CPU_State.r [7] = MEM16 [Device_Data.CPU_State.r [6] >> 1];
        Device_Data.CPU_State.r [6] += 2;
    }
}

void main_program(void)
{
    int_fast32_t  Time;
    uint_fast32_t T            = 0;
    uint_fast16_t CodeAndFlags;
    uint_fast16_t Key;
    uint_fast32_t LastKey      = 0xC00;
    uint_fast8_t  RunState     = 0;
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

                if (Device_Data.CPU_State.r [7] == 0116076)
                {
                    TapeOp ();

                    Time = getCycleCount ();
                    T    = Device_Data.CPU_State.Time;
                }
            }

            NewT = getCycleCount ();
            if ((int32_t) (NewT - Time) > 0x10000L) Time = NewT - 0x10000L;
        }

        // Вся периодика

        switch (RunState++)
        {
            default:
            case 0:
                CPU_TimerRun ();
                RunState = 1;
                break;

            case 1:
                ps2_periodic ();
                break;

            case 2:
                CodeAndFlags = ps2_read ();
                if (CodeAndFlags == 0) RunState = 5;
                break;

            case 3:
                Key = Key_Translate (CodeAndFlags);
                break;

            case 4:
                ps2_leds ((Key_Flags >> KEY_FLAGS_CAPSLOCK_POS) & 1, (Key_Flags >> KEY_FLAGS_NUMLOCK_POS) & 1, (Key_Flags >> KEY_FLAGS_TURBO_POS) & 1);

                if (Key_Flags & KEY_FLAGS_NUMLOCK) MEM16 [0177714 >> 1] = (uint16_t) (Key_Flags >> KEY_FLAGS_UP_POS);
                else                               MEM16 [0177714 >> 1] = 0;

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

                if (((MEM16 [0177660 >> 1] & 0200) | (LastKey & 0x800)) == 0)
                {
                    Key = (uint_fast16_t) (LastKey >> 16);

                    LastKey |= 0x800;

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
                    MEM16 [0177662 >> 1]  = (uint16_t) Key & 0177;
                }

                RunState = 0;

                break;
        }
    }
}
