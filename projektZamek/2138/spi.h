#ifndef _SPI_H_
#define _SPI_H_

#include <general.h>
#include <lpc2xxx.h>


void SPI_Master_Init();
void SPI_Master_Write(tU8 data);
tU8 SPI_Master_Read();



#endif
