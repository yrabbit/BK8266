#include <stdio.h>
#include <stdint.h>

uint16_t ROM [0x4000] =
{
	#include "rom.h"
};

void main (void)
{
	FILE *pFile = fopen ("rom.bin", "wb");

	fwrite (ROM, sizeof (ROM), 1, pFile);
}
