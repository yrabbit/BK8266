#ifndef ETS_H
#define ETS_H


#include <inttypes.h>


extern void uart_div_modify(int no, unsigned int freq);

extern void gpio_init(void);
extern void gpio_output_set(uint32_t set_mask,
                     uint32_t clear_mask,
                     uint32_t enable_mask,
                     uint32_t disable_mask);
extern uint32_t gpio_input_get(void);

typedef struct
{
	uint32_t  deviceId;
	uint32_t  chip_size;    // chip size in byte
	uint32_t  block_size;
	uint32_t  sector_size;
	uint32_t  page_size;
	uint32_t  status_mask;
} SpiFlashChip;
extern SpiFlashChip *flashchip;
typedef int SpiFlashOpResult;
extern SpiFlashOpResult SPI_read_status	 (SpiFlashChip *sflashchip, uint32_t *sta);
extern SpiFlashOpResult SPI_write_status (SpiFlashChip *sflashchip, uint32_t sta);
extern SpiFlashOpResult SPI_write_enable (SpiFlashChip *sflashchip);
extern SpiFlashOpResult Wait_SPI_Idle	 (SpiFlashChip *sflashchip);
extern uint32_t 		SPIRead			 (uint32_t addr, void *outptr, uint32_t len);
extern uint32_t 		SPIEraseSector	 (int);
extern uint32_t 		SPIWrite		 (uint32_t addr, const void *inptr, uint32_t len);

extern void ets_printf(char*, ...);
extern void ets_delay_us(int);
extern void ets_memset(void*, uint8_t, uint32_t);
extern void ets_memcpy(void*, const void*, uint32_t);
extern int ets_memcmp(const void *s1, const void *s2, uint32_t n);
extern uint8_t ets_get_cpu_frequency(void);

extern void rom_i2c_writeReg(uint32_t block, uint32_t host_id, uint32_t reg_add, uint32_t data);
extern void rom_i2c_writeReg_Mask(uint32_t block, uint32_t host_id, uint32_t reg_add, uint32_t Msb, uint32_t Lsb, uint32_t indata);
extern uint8_t rom_i2c_readReg_Mask(uint32_t block, uint32_t host_id, uint32_t reg_add, uint32_t Msb, uint32_t Lsb);
extern uint8_t rom_i2c_readReg(uint32_t block, uint32_t host_id, uint32_t reg_add);


#define ETS_UNCACHED_ADDR(addr) (addr)
#define READ_PERI_REG(addr) (*((volatile uint32_t *)ETS_UNCACHED_ADDR(addr)))
#define WRITE_PERI_REG(addr, val) (*((volatile uint32_t *)ETS_UNCACHED_ADDR(addr))) = (uint32_t)(val)
#define CLEAR_PERI_REG_MASK(reg, mask) WRITE_PERI_REG((reg), (READ_PERI_REG(reg)&(~(mask))))
#define SET_PERI_REG_MASK(reg, mask)   WRITE_PERI_REG((reg), (READ_PERI_REG(reg)|(mask)))
#define GET_PERI_REG_BITS(reg, hipos,lowpos)      ((READ_PERI_REG(reg)>>(lowpos))&((1<<((hipos)-(lowpos)+1))-1))
#define SET_PERI_REG_BITS(reg,bit_map,value,shift) (WRITE_PERI_REG((reg),(READ_PERI_REG(reg)&(~((bit_map)<<(shift))))|((value)<<(shift)) ))


#endif
