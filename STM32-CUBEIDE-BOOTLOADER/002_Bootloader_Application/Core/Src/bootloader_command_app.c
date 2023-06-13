/*
 * bootloader_command_app.c
 *
 *  Created on: 8 Jun 2023
 *      Author: Mustafa
 */
#include "bootloader_command_app.h"
extern uint8_t supported_commands[];
/*                        CRC VERIFICATION AREA        */
uint8_t bootloader_verify_crc(uint8_t *buffer,uint32_t length,uint32_t crc_host)
{
  uint32_t crc_Value = 0xFF;
  uint32_t data = 0;
  for(uint32_t i = 0;i<length;i++)
  {
	  data = buffer[i];
	  crc_Value = HAL_CRC_Accumulate(&hcrc,&data,1);

  }
  __HAL_CRC_DR_RESET(&hcrc);
  if(crc_Value == crc_host)
  {
    return CRC_SUCCESS;

  }

 return CRC_FAIL;
}
void bootloader_send_ack(uint8_t follow_length)
{
  uint8_t ackBuffer[2];
  ackBuffer[0] = BL_ACK_VALUE;
  ackBuffer[1] = follow_length;
  HAL_UART_Transmit(&huart3,ackBuffer,2,HAL_MAX_DELAY);

}
void bootloader_send_nack()
{
	uint8_t nackValue = BL_NACK_VALUE;
	HAL_UART_Transmit(&huart3,&nackValue,1,HAL_MAX_DELAY);

}
void bootloader_uart_write_data(uint8_t *Buffer,uint32_t length)
{
  HAL_UART_Transmit(&huart3,Buffer,length,HAL_MAX_DELAY);
}
uint8_t bootloader_get_version(void)
{
   return BL_VER;
}
void bootloader_get_ver_cmd(uint8_t *bl_rx_data)
{
    uint8_t bl_Version = 0;
    printMessage("BL_DEBUG_MSG: Bootloader Get Version Command Working\r\n");

    uint32_t command_package_length = bl_rx_data[0] + 1; // Total Length of the data

    uint32_t host_crc = *((uint32_t *)(bl_rx_data + command_package_length - 4));

    //CRC CONTROL
    if(!bootloader_verify_crc(&bl_rx_data[0],command_package_length-4,host_crc))
    {
       printMessage("BL_DEBUG_MSG: Checksum success\r\n");
       bootloader_send_ack(1);
       bl_Version = bootloader_get_version();
       printMessage("BL_DEBUG_MSG: BL_VER = %d\r\n",bl_Version);
       bootloader_uart_write_data(&bl_Version,1);

    }
    else
    {
    	printMessage("BL_DEBUG_MSG: Checksum unsuccessful\r\n");
    	bootloader_send_nack();

    }
}
void bootloader_get_help_cmd(uint8_t *bl_rx_data)
{
   printMessage("BL_DEBUG_MSG: bootloader_get_help_cmd\r\n");

   uint32_t command_package_length = bl_rx_data[0] + 1;

   uint32_t host_crc = *((uint32_t *)(bl_rx_data + command_package_length - 4));

   if(!bootloader_verify_crc(&bl_rx_data[0],command_package_length-4,host_crc))
   {
	   printMessage("BL_DEBUG_MSG: Checksum success\r\n");
	   bootloader_send_ack(strlen((char*)supported_commands));
	   bootloader_uart_write_data(supported_commands,strlen((char*)supported_commands));
	   for(int i = 0;i<strlen((char*)supported_commands);i++)
	   {
              printMessage("%#x  ",supported_commands[i]);
	   }
   }
   else
   {
	   printMessage("BL_DEBUG_MSG: Checksum unsuccessful\r\n");
	    bootloader_send_nack();
   }
}
void bootloader_get_cid_cmd(uint8_t *bl_rx_data)
{
	   uint16_t cID = 0;
	   printMessage("BL_DEBUG_MSG: bootloader_get_cid_cmd\r\n");

	   uint32_t command_package_length = bl_rx_data[0] + 1;

	   uint32_t host_crc = *((uint32_t *)(bl_rx_data + command_package_length - 4));

	   if(!bootloader_verify_crc(&bl_rx_data[0],command_package_length-4,host_crc))
	   {
		   printMessage("BL_DEBUG_MSG: Checksum success\r\n");
		   bootloader_send_ack(2);
		   cID = get_mcu_chip_id();
		   printMessage("BL_DEBUG_MSG: Chip ID = %d  %#x\r\n",cID,cID);
		   bootloader_uart_write_data((uint8_t*)&cID,2);

	   }
	   else
	   {
		    printMessage("BL_DEBUG_MSG: Checksum unsuccessful\r\n");
		    bootloader_send_nack();
	   }

}
uint16_t get_mcu_chip_id(void)
{
  uint16_t cID;
  cID = (uint16_t)(DBGMCU->IDCODE) & 0x0FFF;
  return cID;

}
void bootloader_get_rdp_cmd(uint8_t *bl_rx_data)
{
   uint8_t rdpLevel = 0 ;

   printMessage("BL_DEBUG_MSG: bootloader_get_rdp_cmd\r\n");

   uint32_t command_package_length = bl_rx_data[0] + 1;

   uint32_t host_crc = *((uint32_t *)(bl_rx_data + command_package_length - 4));

   if(!bootloader_verify_crc(&bl_rx_data[0],command_package_length-4,host_crc))
   {
	   printMessage("BL_DEBUG_MSG: Checksum success\r\n");
	   bootloader_send_ack(1);
	   rdpLevel = get_flash_rdp_level();
	   printMessage("BL_DEBUG_MSG: STM32F4 RDP LEVEL: %d %#x\r\n",rdpLevel,rdpLevel);
	   bootloader_uart_write_data(&rdpLevel,1);


   }
   else
   {
	    printMessage("BL_DEBUG_MSG: Checksum unsuccessful\r\n");
	    bootloader_send_nack();
   }

}
uint8_t get_flash_rdp_level(void)
{
  uint8_t rdp_level = 0;

  volatile uint32_t *OB_Address = (uint32_t *)0x1FFFC000;
  rdp_level = (uint8_t)(*OB_Address >> 8);

  return rdp_level;
}
void bootloader_go_to_addr_cmd(uint8_t *bl_rx_data)
{
	uint32_t go_to_address = 0;
	uint8_t adrr_valid =   ADD_VALID;
	uint8_t adrr_invalid = ADD_INVALID;

	printMessage("BL_DEBUG_MSG: bootloader_go_to_addr_cmd\r\n");

	uint32_t command_package_length = bl_rx_data[0] + 1;

	uint32_t host_crc = *((uint32_t *)(bl_rx_data + command_package_length - 4));

	   if(!bootloader_verify_crc(&bl_rx_data[0],command_package_length-4,host_crc))
	   {
		   printMessage("BL_DEBUG_MSG: Checksum success\r\n");
		   bootloader_send_ack(1);

		   go_to_address = *(uint32_t *)&bl_rx_data[2];
		   printMessage("BL_DEBUG_MSG: GO ADDRESS = %#x\r\n",go_to_address);

		   if(bootloader_verify_addr(go_to_address) == ADD_VALID)
		   {
             bootloader_uart_write_data(&adrr_valid,1);

             // Go to the Address
             go_to_address +=1; // T Bit = 1
             void (*lets_go_to_address)(void) = (void*)go_to_address;
             printMessage("BL_DEBUG_MSG: GOIN TO ADDRESS");
             lets_go_to_address();


		   }
		   else
		   {
			  printMessage("BL_DEBUG_MSG: Go Address INVALID\r\n");
			  bootloader_uart_write_data(&adrr_invalid,1);
		   }

	   }
	   else
	   {
		    printMessage("BL_DEBUG_MSG: Checksum unsuccessful\r\n");
		    bootloader_send_nack();
	   }


}
uint8_t bootloader_verify_addr(uint32_t goaddr)
{

  if(goaddr >= FLASH_BASE && goaddr <= FLASH_END)
  {
	  return ADD_VALID;
  }
  else if(goaddr >= SRAM1_BASE && goaddr <= SRAM1_END)
  {
	  return ADD_VALID;

  }
  else if(goaddr >= SRAM2_BASE && goaddr <= SRAM2_END)
  {
	  return ADD_VALID;

  }
  else if(goaddr >= SRAM3_BASE && goaddr <= SRAM3_END)
  {
	  return ADD_VALID;

  }
  else if(goaddr >= BKPSRAM_BASE && goaddr <= BKPSRAM_END)
  {
	  return ADD_VALID;

  }
  else if(goaddr >= FLASH_OTP_BASE && goaddr <= FLASH_OTP_END)
  {
	  return ADD_VALID;

  }

  else
  {
   return ADD_INVALID;
  }

}
void bootloader_flash_erase_cmd(uint8_t *bl_rx_data)
{
	   uint8_t eraseStatus = 0;

	   printMessage("BL_DEBUG_MSG: bootloader_flash_erase_cmd\r\n");

	   uint32_t command_package_length = bl_rx_data[0] + 1;

	   uint32_t host_crc = *((uint32_t *)(bl_rx_data + command_package_length - 4));

	   if(!bootloader_verify_crc(&bl_rx_data[0],command_package_length-4,host_crc))
	   {
		   printMessage("BL_DEBUG_MSG: Checksum success\r\n");
		  bootloader_send_ack(1);
		  printMessage("BL_DEBUG_MSG: Initial sector: %d Number of Sector : %d\r\n",bl_rx_data[2],bl_rx_data[3]);

		  eraseStatus = execute_flash_erase(bl_rx_data[2],bl_rx_data[3]);

		  printMessage("BL_DEBUG_MSG: Flash Erase Status: %d  \r\n",eraseStatus);
		  bootloader_uart_write_data(&eraseStatus,1);

	   }
	   else
	   {
		   printMessage("BL_DEBUG_MSG: Checksum unsuccessful\r\n");
		    bootloader_send_nack();
	   }


}
uint8_t execute_flash_erase(uint8_t start_sector,uint8_t number_of_sector)
{
	uint8_t erase_status = 0;
	FLASH_EraseInitTypeDef FLASH_EraseInitStruct = {0};
	uint32_t SectorError = 0;

	if(number_of_sector > 11)
	{
		return INVALID_SECTOR;

	}

	if((start_sector<=11) || start_sector == 0xFF)
	{
      if(start_sector == 0xFF)
      {
    	  // Mass Erase
    	  FLASH_EraseInitStruct.TypeErase = FLASH_TYPEERASE_MASSERASE;

      }
      else
      {
    	  // Sector Erase
    	  uint8_t remaining_sector = 11 - start_sector;
    	  if(start_sector > remaining_sector)
    	  {
    		  start_sector = remaining_sector;
    	  }
    	  FLASH_EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    	  FLASH_EraseInitStruct.Sector = start_sector;
    	  FLASH_EraseInitStruct.NbSectors = number_of_sector;
      }
      FLASH_EraseInitStruct.Banks = FLASH_BANK_BOTH;

      HAL_FLASH_Unlock();
      FLASH_EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
      erase_status = (uint8_t) HAL_FLASHEx_Erase(&FLASH_EraseInitStruct,&SectorError);
      HAL_FLASH_Lock();
      return erase_status;

	}

    return INVALID_SECTOR;

}
void bootloader_mem_write_cmd(uint8_t *bl_rx_data)
{

       uint8_t write_status = 0x00;
       uint8_t checkSum = 0,length = 0;

       length = bl_rx_data[0];
       uint8_t payload_length = bl_rx_data[6];
       uint32_t memory_addr = *((uint32_t *)(&bl_rx_data[2]));

       checkSum = bl_rx_data[length];

       printMessage("BL_DEBUG_MSG: bootloader_mem_write_cmd\r\n");

	   uint32_t command_package_length = bl_rx_data[0] + 1;

	   uint32_t host_crc = *((uint32_t *)(bl_rx_data + command_package_length - 4));

	   if(!bootloader_verify_crc(&bl_rx_data[0],command_package_length-4,host_crc))
	   {
		  printMessage("BL_DEBUG_MSG: Checksum success\r\n");
		  bootloader_send_ack(1);
		  printMessage("BL_DEBUG_MSG: Memory Write Address : %#x\r\n",memory_addr);
		  if(bootloader_verify_addr(memory_addr) == ADD_VALID)
		  {
			  printMessage("BL_DEBUG_MSG: Valid Memory Write Address\r\n");
			  write_status = execute_mem_write(&bl_rx_data[7],memory_addr,payload_length);

		  }
		  else
		  {
			  printMessage("BL_DEBUG_MSG: Invalid Memory Write Address\r\n");
			  write_status = ADD_INVALID;
			  bootloader_uart_write_data(&write_status,1);
		  }

	   }
	   else
	   {
		    printMessage("BL_DEBUG_MSG: Checksum unsuccessful\r\n");
		    bootloader_send_nack();
	   }

}
uint8_t execute_mem_write(uint8_t *Buffer,uint32_t memAddress,uint32_t len)
{
    uint8_t status = 0;

    HAL_FLASH_Unlock();

    for(uint32_t i = 0; i<len;i++)
    {
    	status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,memAddress+i,Buffer[i]);

    }
    HAL_FLASH_Lock();

    return status;

}
void bootloader_en_rw_protect_cmd(uint8_t *bl_rx_data)
{
       uint8_t status = 0;

       printMessage("BL_DEBUG_MSG: bootloader_en_rw_protect_cmd\r\n");

	   uint32_t command_package_length = bl_rx_data[0] + 1;

	   uint32_t host_crc = *((uint32_t *)(bl_rx_data + command_package_length - 4));

	   if(!bootloader_verify_crc(&bl_rx_data[0],command_package_length-4,host_crc))
	   {
		  printMessage("BL_DEBUG_MSG: Checksum success\r\n");
		  bootloader_send_ack(1);

		  status = configure_flash_sector_r_w_protection(bl_rx_data[2],bl_rx_data[3],0);
		  printMessage("BL_DEBUG_MSG: Flash Protect Status: %d",status);
		  bootloader_uart_write_data(&status,1);


	   }
	   else
	   {
		    printMessage("BL_DEBUG_MSG: Checksum unsuccessful\r\n");
		    bootloader_send_nack();

	   }


}
void bootloader_dis_rw_protect_cmd(uint8_t *bl_rx_data)
{
       uint8_t status = 0;

       printMessage("BL_DEBUG_MSG: bootloader_dis_rw_protect_cmd\r\n");

	   uint32_t command_package_length = bl_rx_data[0] + 1;

	   uint32_t host_crc = *((uint32_t *)(bl_rx_data + command_package_length - 4));

	   if(!bootloader_verify_crc(&bl_rx_data[0],command_package_length-4,host_crc))
	   {
		   printMessage("BL_DEBUG_MSG: Checksum success\r\n");
		   bootloader_send_ack(1);

		   status = configure_flash_sector_r_w_protection(0,0,1);
		   printMessage("BL_DEBUG_MSG: Flash Protect Status: %d",status);
		   bootloader_uart_write_data(&status,1);


	   }
	   else
	   {
		    printMessage("BL_DEBUG_MSG: Checksum unsuccessful\r\n");
		    bootloader_send_nack();

	   }

}
uint8_t configure_flash_sector_r_w_protection(uint8_t sector_detail,uint8_t protection_mode,uint8_t enordisable)
{

    //EnableorDisable == 0 -> en_w_r_protect ||EnableorDisable == 1 -> dis_w_r_protect
    if(enordisable)
    {
       	HAL_FLASH_OB_Unlock();
        while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);

        FLASH->OPTCR |= (0xFF<<16);

    	FLASH->OPTCR |= (1<<1);

    	while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);

    	HAL_FLASH_OB_Lock();
    	return 0;

    }
    if(protection_mode == 1) //Only Write Protection
    {
    	HAL_FLASH_OB_Unlock();
    	while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);
    	FLASH->OPTCR &= ~(sector_detail << 16);

    	FLASH->OPTCR |= (1<<1);

    	while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);

    	HAL_FLASH_OB_Lock();

    	return 1;

    }
    else if(protection_mode == 2)// Read Write Protection
    {
    	HAL_FLASH_OB_Unlock();
    	while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);

    	FLASH->OPTCR &= ~(0xFF << 16);

    	FLASH->OPTCR |= (sector_detail<<16);

    	FLASH->OPTCR |= (1<<1);

    	while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);

    	HAL_FLASH_OB_Lock();
    	return 1;



    }

    return 1;


}
void bl_read_sector_p_status_cmd(uint8_t *bl_rx_data)
{
       uint16_t status = 0;

       printMessage("BL_DEBUG_MSG: bl_read_sector_p_status_cmd\r\n");

	   uint32_t command_package_length = bl_rx_data[0] + 1;

	   uint32_t host_crc = *((uint32_t *)(bl_rx_data + command_package_length - 4));

	   if(!bootloader_verify_crc(&bl_rx_data[0],command_package_length-4,host_crc))
	   {
		  printMessage("BL_DEBUG_MSG: Checksum success\r\n");
		  bootloader_send_ack(1);
		  status = read_ob_r_w_protection_status();
		  printMessage("BL_DEBUG_MSG: nWRP Status: %#x \r\n",status);
		  bootloader_uart_write_data((uint8_t*)&status,2);

	   }
	   else
	   {
		    printMessage("BL_DEBUG_MSG: Checksum unsuccessful\r\n");
		    bootloader_send_nack();

	   }



}
uint16_t read_ob_r_w_protection_status()
{
  FLASH_OBProgramInitTypeDef flashOBInitStruct;

  HAL_FLASH_OB_Unlock();
  HAL_FLASHEx_OBGetConfig(&flashOBInitStruct);
  HAL_FLASH_OB_Lock();

  return flashOBInitStruct.WRPSector;

}
