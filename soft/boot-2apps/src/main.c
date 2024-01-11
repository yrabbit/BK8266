#include "ets.h"
#include "check_image.h"
#include "ram_loader.h"
#include "fw_update.h"


uint32_t main_program(void)
{
    uint32_t runAddr;

    if(rom_i2c_readReg(103,4,1) != 136) // 8: 40MHz, 136: 26MHz
    {
        //if(get_sys_const(sys_const_crystal_26m_en) == 1) // soc_param0: 0: 40MHz, 1: 26MHz, 2: 24MHz
        {
            // set 80MHz PLL CPU
            rom_i2c_writeReg(103,4,1,0x88);
            rom_i2c_writeReg(103,4,2,0x91);
        }
    }

    // ������ UART �� 115200
    uart_div_modify(0, 80000000/115200);
    ets_delay_us(100000);

    ets_printf("\r\nRK8266 Boot\r\n");

    // ���� ���� - ��������� ��������
    fw_update();

    // ��������� - ����� ��������� ���� ���������
    if (READ_PERI_REG(0x60001200) != 0x55AA55AA)
    {
	// ��������

    ets_printf("Emu\r\n");
	// ��������� ���, ��� ���� +32�� IRAM
	CLEAR_PERI_REG_MASK(0x3ff00024, 0x00000018);

	runAddr=check_image(0x01000);
    } else
    {
	// WiFi
    ets_printf("WiFi\r\n");
	runAddr=check_image(0x10000);
    }

    if (runAddr==0)
    {
	ets_printf("BAD ROM !!!\r\n");
	return 0;
    }

    ets_printf("Booting rom...\r\n");
    // copy the loader to top of iram
    ets_memcpy((void*)_text_addr, _text_data, _text_len);
    // return address to load from
    return runAddr;

}


// assembler stub uses no stack space
// works with gcc
void call_user_start(void) {
	__asm volatile (
		"mov a15, a0\n"          // store return addr, hope nobody wanted a15!
		"call0 main_program\n"     // find a good rom to boot
		"mov a0, a15\n"          // restore return addr
		"bnez a2, 1f\n"          // ?success
		"ret\n"                  // no, return
		"1:\n"                   // yes...
		"movi a3, entry_addr\n"  // get pointer to entry_addr
		"l32i a3, a3, 0\n"       // get value of entry_addr
		"jx a3\n"                // now jump to it
	);
}
