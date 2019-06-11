#include "rfid.h"

void writeMFRC522(tU8 addr, tU8 val)
{
   IOCLR = (1 << 7)
		; // SSEL = 0, enable SPI communication with slave
   SPI_Master_Write((addr << 1) & 0x7E)
		; // Send  register address	
   SPI_Master_Write(val)
		; // Write value to register under sended address	
   IOSET = (1 << 7)
		; // SSEL = 1, disable SPI communication with slave
}


tU8 readMFRC522(tU8 addr)
{
   tU8 val
		; // Keep received data
   IOCLR = (1 << 7)
		; // SSEL = 0, enable SPI communication with slave
   SPI_Master_Write(((addr << 1) & 0x7E) | 0x80)
		; // Send register address 0x80 (to read data from the MFRC522 the MSB must be set to logic 1)
   val = SPI_Master_Read()
		; // Read data from SPI
   IOSET = (1 << 7)
		; // SSEL = 1, disable SPI communication with slave
   return val;   
}

void reset()
{
   writeMFRC522(CommandReg, PCD_RESETPHASE)
		; // Reset MFRC522
}


void setBitMask(tU8 reg, tU8 mask)  
{ 
    tU8 tmp;
    tmp = readMFRC522(reg);
    writeMFRC522(reg, tmp | mask)
		; // Set bit mask
}


void clearBitMask(tU8 reg, tU8 mask)  
{
    tU8 tmp;
    tmp = readMFRC522(reg);
    writeMFRC522(reg, tmp & (~mask))
		;  // Clear bit mask
} 


void antennaOn()
{
   tU8 temp;

   temp = readMFRC522(TxControlReg);
   if (!(temp & 0x03))
   {
      setBitMask(TxControlReg, 0x03)
		; // First two bits from the TxControlReg must be set to logic 1
   }
}


tU8 MFRC522ToCard(tU8 command, tU8 *sendData, tU8 sendLen, tU8 *backData, tU32 *backLen)
{
    tU8 status = MI_ERR;
    tU8 irqEn = 0x77;
    tU8 waitIRq = 0x30;
    tU8 lastBits;
    tU8 n;
    tU32 i;

    writeMFRC522(CommIEnReg, irqEn  | 0x80);
    clearBitMask(CommIrqReg, 0x80);
    setBitMask(FIFOLevelReg, 0x80);

   writeMFRC522(CommandReg, PCD_IDLE);

    for (i = 0; i < sendLen; i++)
    {   
      writeMFRC522(FIFODataReg, sendData[i]);    
    }

   writeMFRC522(CommandReg, command)
		; // Transmits data from FIFO buffer to antenna and automatically activates the receiver after transmission
    
   setBitMask(BitFramingReg, 0x80)
		; // Starts the transmission of data only valid in combination with the Transceive command

   i = 2000;
    do 
    {
        n = readMFRC522(CommIrqReg);
        i--;
    }
    while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

    clearBitMask(BitFramingReg, 0x80)
		;  // StartSend = 0

    if (i != 0)
    {    
        if(!(readMFRC522(ErrorReg) & 0x1B))
        {
            
			status = MI_OK;
            
			if (n & irqEn & 0x01)
            {   
            status = MI_NOTAGERR;
			}

             n = readMFRC522(FIFOLevelReg)
				; // Number of bytes stored in the FIFO buffer
             lastBits = readMFRC522(ControlReg) & 0x07;
            
			if (lastBits)
            {   
               *backLen = (n - 1) * 8 + lastBits;   
			}	else {   
                  *backLen = n*8;   
				}

                if (n == 0)
                {   
					n = 1;    
				}
				
                if (n > MAX_LEN)
                {   
					n = MAX_LEN;   
				}

                for (i=0; i < n; i++)
                {   
					backData[i] = readMFRC522(FIFODataReg);    
				}
    
        }
        else	{   
         status = MI_ERR;  
      }

    }

    return status;
}



tU8  MFRC522Request(tU8 reqMode, tU8 *TagType)
{
   tU8 status;  
   tU32 backBits;

   writeMFRC522(BitFramingReg, 0x07);

   TagType[0] = reqMode;
   
   status = MFRC522ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

   if ((status != MI_OK) || (backBits != 0x10))
   {    
      status = MI_ERR;
   }

   return status;
}



tU8 anticoll(tU8 *serNum)
{
    tU8 status;
    tU8 i;
    tU8 serNumCheck = 0;
    tU32 unLen;
   writeMFRC522(BitFramingReg, 0x00);

    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = MFRC522ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if (status == MI_OK)
   {
      for (i = 0; i < 4; i++)
      {   
          serNumCheck ^= serNum[i];
      }
      if (serNumCheck != serNum[i])
      {   
         status = MI_ERR;    
      }
    }

    return status;
}



tU8 isCard()
 {
   tU8 status;  
   tU8 str[MAX_LEN];

   status = MFRC522Request(PICC_REQIDL, str);
   
   if (status == MI_OK) {
      return 1;
   } else { 
      return 0; 
   }
   
 }
 

tU8 readCardSerial() 
{
   tU8 status;
   tU8 str[MAX_LEN];

   status = anticoll(str);
   
   static tU8 i;
   for (i = 0; i < 4; i++) {
	   *(serNum + i) = *(str + i);
   }

   if (status == MI_OK) {
      return 1;
   } else {
      return 0;
   }

 }

tU8 checkIfValidTag() {
	tU8 validTag[4] = {0xe7, 0x92, 0xde, 0x03};
	tU8 i;
	for (i = 0; i < 4; i++) {
		if(serNum[i] != validTag[i]) {
			return FALSE;
		}
	}
	for (i = 0; i < 4; i++){
		serNum[i] = 0x00;
	}
	return TRUE;
}


void initRFID()
{
 
   reset()
		; // Soft reset MFRC522
   writeMFRC522(TModeReg, 0x8D)
		; // timer starts automatically D - TPreScaler
   writeMFRC522(TPrescalerReg, 0x3E)
		; // TModeReg[3..0] + TPrescalerReg defines the lower 8 bits of the TPrescaler value
   writeMFRC522(TReloadRegL, 30);           
   
   writeMFRC522(TReloadRegH, 0);

   writeMFRC522(TxAutoReg, 0x40)
		;	// 100%ASK
   antennaOn()
		; 	// Check and turn on antenna
}
