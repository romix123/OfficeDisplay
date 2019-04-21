
#define romix_background 0xF79E //0xEFEBE9//0xecf0f1
#define romix_heading    0x0086 //0x000e25
#define romix_content    0x7CB4 //0x68828e
#define romix_separation 0xE73D  //0xd8dfe2
#define romix_accent     0x04B9 //0x0083be
#define romix_red        0x4180 // f44336
#define romix_green      0xB9A1 // 0x8bc34a

void doIcon(String icon, int i){
int y;
  if (i ==1) y = 28; else y = 149;
  int x = 248;

    //"home, busy, travel, teach, sick, telco, meeting, welcome, sport"
    if (icon == "-home") {
          tft.drawRGBBitmap(x, y, homeL, 75, 75);
    } else if (icon == "-busy") {
          tft.drawRGBBitmap(x, y, busy, 75, 75);
    } else if (icon == "-travel") {
          tft.drawRGBBitmap(x, y, globe, 75,75);
//    } else if (icon == "-teach") {
//          tft.drawRGBBitmap(x, y, teach, 75, 75);
    } else if (icon == "-sick") {
          tft.drawRGBBitmap(x, y, sick, 75, 74);
  } else if (icon == "-telco") {
          tft.drawRGBBitmap(x, y, confcallF, 75, 75);          
    } else if (icon == "-meeting") {
          tft.drawRGBBitmap(x, y, meeting, 75, 75);
    } else if (icon == "-welcome") {
          tft.drawRGBBitmap(x, y, faq, 75, 75);
    } else if (icon == "-sport") {
          tft.drawRGBBitmap(x, y, sport, 75, 75);
    }      
}

void drawOccupant1(){
   tft.fillRect(0,0, 320, 25, romix_accent);
   tft.setCursor(3,16);
   tft.setFont(&FreeSansBold12pt7b);
   tft.setTextColor(romix_background);
//  tft.setTextColor(romix_accent); 
   tft.print(occupantS1);
}

void drawOccupant2(){
   tft.fillRect(0,121, 320, 25, romix_accent);
   tft.setTextColor(romix_background);  
   tft.setFont(&FreeSansBold12pt7b);
 //  tft.setTextColor(romix_accent);  
   tft.setCursor(3,141);
   tft.print(occupantS2);  
}

void drawdisplay(){
   tft.fillScreen(romix_background);
  drawOccupant1();
  if (occupantS2 != "" ){
      tft.drawLine(0,120,320,120, romix_separation);
      drawOccupant2();
  }
//   tft.setCursor(5,42);
//    tft.setFont(&FreeSans9pt7b);
//    tft.setTextColor(romix_content);  tft.setTextSize(1);
  }

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void drawMessage(String msg){
     int i, j = 0;
     int lines = 1;
     int maxlines;
     (occupantS2 =="") ? maxlines = 9 : maxlines = 4;
     String returnstring;
     
    String nextWord;
       nextWord = getValue(msg, ' ',i);
    while (nextWord != "" && lines < maxlines) {
      if (j + nextWord.length() > 28) {
        tft.println();
        Serial.println();
        j = 0;
        lines ++;
       } 
        tft.print(" ");
        Serial.print(" ");
        Serial.print(nextWord);
        tft.print(nextWord);
        //returnstring += " " + nextWord;
        j = j + nextWord.length() + 1;
        i++;
        nextWord = getValue(msg, ' ',i);
    }
}
