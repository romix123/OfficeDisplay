/*
 * Flash stuff from EPSEasy
 */


#if FEATURE_SPIFFS
/********************************************************************************************\
  Save data into config file on SPIFFS
  \*********************************************************************************************/
void SaveToFile(char* fname, int index, byte* memAddress, int datasize)
{
  File f = SPIFFS.open(fname, "r+");
  if (f)
  {
    f.seek(index, SeekSet);
    byte *pointerToByteToSave = memAddress;
    for (int x = 0; x < datasize ; x++)
    {
      f.write(*pointerToByteToSave);
      pointerToByteToSave++;
    }
    f.close();
    Serial.println("FILE : File saved");
  }
}


/********************************************************************************************\
  Load data from config file on SPIFFS
  \*********************************************************************************************/
void LoadFromFile(char* fname, int index, byte* memAddress, int datasize)
{
  File f = SPIFFS.open(fname, "r+");
  if (f)
  {
    f.seek(index, SeekSet);
    byte *pointerToByteToRead = memAddress;
    for (int x = 0; x < datasize; x++)
    {
      *pointerToByteToRead = f.read();
      pointerToByteToRead++;// next byte
    }
    f.close();
  }
}
#endif


/********************************************************************************************\
  Save data to flash
  \*********************************************************************************************/
void SaveToFlash(int index, byte* memAddress, int datasize)
{
  if (index > 33791) // Limit usable flash area to 32+1k size
  {
    return;
  }
  uint32_t _sector = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint8_t* data = new uint8_t[FLASH_EEPROM_SIZE];
  int sectorOffset = index / SPI_FLASH_SEC_SIZE;
  int sectorIndex = index % SPI_FLASH_SEC_SIZE;
  uint8_t* dataIndex = data + sectorIndex;
  _sector += sectorOffset;

  // load entire sector from flash into memory
  noInterrupts();
  spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE);
  interrupts();

  // store struct into this block
  memcpy(dataIndex, memAddress, datasize);

  noInterrupts();
  // write sector back to flash
  if (spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK)
    if (spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE) == SPI_FLASH_RESULT_OK)
    {
      //Serial.println("flash save ok");
    }
  interrupts();
  delete [] data;
  Serial.println("FLASH: Settings saved");

}


/********************************************************************************************\
  Load data from flash
  \*********************************************************************************************/
void LoadFromFlash(int index, byte* memAddress, int datasize)
{
  uint32_t _sector = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint8_t* data = new uint8_t[FLASH_EEPROM_SIZE];
  int sectorOffset = index / SPI_FLASH_SEC_SIZE;
  int sectorIndex = index % SPI_FLASH_SEC_SIZE;
  uint8_t* dataIndex = data + sectorIndex;
  _sector += sectorOffset;

  // load entire sector from flash into memory
  noInterrupts();
  spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE);
  interrupts();

  // load struct from this block
  memcpy(memAddress, dataIndex, datasize);
  delete [] data;
}


/********************************************************************************************\
  Erase data on flash
  \*********************************************************************************************/
void ZeroFillFlash()
{
  // this will fill the SPIFFS area with a 64k block of all zeroes.
  uint32_t _sectorStart = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint32_t _sectorEnd = _sectorStart + 16 ; //((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint8_t* data = new uint8_t[FLASH_EEPROM_SIZE];

  uint8_t* tmpdata = data;
  for (int x = 0; x < FLASH_EEPROM_SIZE; x++)
  {
    *tmpdata = 0;
    tmpdata++;
  }


  for (uint32_t _sector = _sectorStart; _sector < _sectorEnd; _sector++)
  {
    // write sector to flash
    noInterrupts();
    if (spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK)
      if (spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE) == SPI_FLASH_RESULT_OK)
      {
        interrupts();
        Serial.print(F("FLASH: Zero Fill Sector: "));
        Serial.println(_sector);
        delay(10);
      }
  }
  interrupts();
  delete [] data;
}
/********************************************************************************************\
  Save settings to SPIFFS
  \*********************************************************************************************/
void SaveSettings(void)
{
#if FEATURE_SPIFFS
  SaveToFile((char*)"config.txt", 0, (byte*)&UserSettings, sizeof(struct UserStruct));
  SaveToFile((char*)"security.txt", 0, (byte*)&SecuritySettings, sizeof(struct SecurityStruct));
#else
  SaveToFlash(0, (byte*)&UserSettings, sizeof(struct UserStruct));
  SaveToFlash(32768, (byte*)&SecuritySettings, sizeof(struct SecurityStruct));
#endif
}


/********************************************************************************************\
  Load settings from SPIFFS
  \*********************************************************************************************/
boolean LoadSettings()
{
#if FEATURE_SPIFFS
  LoadFromFile((char*)"config.txt", 0, (byte*)&UserSettings, sizeof(struct UserStruct));
  LoadFromFile((char*)"security.txt", 0, (byte*)&SecuritySettings, sizeof(struct SecurityStruct));
#else
  LoadFromFlash(0, (byte*)&UserSettings, sizeof(struct UserStruct));
  LoadFromFlash(32768, (byte*)&SecuritySettings, sizeof(struct SecurityStruct));

  botToken = UserSettings.botToken;
  occupantS1 = UserSettings.occupant1;
  occupantS2 = UserSettings.occupant2;
  ownerS1   = UserSettings.owner1;
  ownerS2   = UserSettings.owner2;
/*
 * 
 */
#endif
}
