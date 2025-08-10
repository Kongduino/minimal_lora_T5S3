#pragma once
typedef struct {
  void (*ptr)(void); // Function pointer
  int x;
  int y;
  char label[5];
  char value[10];
} kButton;
void drawButton(kButton, uint8_t);

kButton myButtons[] = {
  { handleLoRa, 0, 0, "LoRa", "" },
  { handleMode, 0, 0, "Fn", "//\\+" },
  { handlePing, 0, 0, "ping", "" },
  { handleBL, 0, 0, "BL", "" },
  { handleMsg, 0, 0, "msg", "" },
  { handleOff, 0, 0, "off", "" },
};

kButton loRaButtons[] = {
  { handleSF, 0, 0, "SF", "11" },
  { handleBW, 0, 0, "BW", "125" },
  { handleFreq, 0, 0, "Frq", "902.6875" },
  { handleCR, 0, 0, "C/R", "8" },
  { handleMain, 0, 0, "Back", "" },
};

kButton sfButtons[] = {
  { handleSF10, 0, 0, "10", "" },
  { handleSF11, 0, 0, "11", "*" },
  { handleSF12, 0, 0, "12", "" },
  { handleLoRa, 0, 0, "Back", "" },
};

kButton bwButtons[] = {
  { handleBW62, 0, 0, "62.5", "" },
  { handleBW125, 0, 0, "125", "*" },
  { handleBW250, 0, 0, "250", "" },
  { handleBW500, 0, 0, "500", "" },
  { handleLoRa, 0, 0, "Back", "" },
};

kButton crButtons[] = {
  { handleCR5, 0, 0, "5", "" },
  { handleCR6, 0, 0, "6", "" },
  { handleCR7, 0, 0, "7", "" },
  { handleCR8, 0, 0, "8", "*" },
  { handleLoRa, 0, 0, "Back", "" },
};

kButton *currButtons = myButtons;
int level0, level1, level2;

void mainMenu(void *x) {
  clearArea(0, minPY, epd_width(), 600);
  currButtons = myButtons;
  btnCount = sizeof(myButtons) / sizeof(kButton);
  baseLine = drawMenu(minPX, minPY);
  level0 = minPY;
  level1 = baseLine + 40;
  level2 = level1 + 110;
  showLoraSettings();
  // showBattery();
  epd_poweron();
  checkError(epd_hl_update_screen(&hl, MODE_GL16, temperature));
  epd_poweroff();
}

void handleOff() {
  char txt[32];
  sprintf(txt, (char *)"Going into deep sleep");
  Serial.println(txt);
  displayStatus(txt);
  delay(500);
  epd_deinit();
  esp_deep_sleep_start();
}

void handleMain() {
  mainMenu(NULL);
}

void handleMode() {
  mainMenu(NULL);
}

void handleLoRa() {
  clearArea(0, level1, epd_width(), 260);
  currButtons = loRaButtons;
  btnCount = sizeof(loRaButtons) / sizeof(kButton);
  int x = drawMenu(10, level1);
}

void setSF() {
  // set spreading factor
  if (radio.setSpreadingFactor(mySF) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
    printf("Selected spreading factor is invalid for this module!\n");
  }
  char txt[32];
  sprintf(txt, (char *)"SF set to %d.", mySF);
  displayStatus(txt);
  sprintf(loRaButtons[0].value, "%d", mySF);
}

void handleSF() {
  printf("handleSF\n");
  clearArea(0, level2, epd_width(), 130);
  currButtons = sfButtons;
  btnCount = sizeof(sfButtons) / sizeof(kButton);
  int x = drawMenu(10, level2);
}

void handleSF11() {
  printf("handleSF11\n");
  mySF = 11;
  setSF();
  displayStatus((char *)"SF set to 11.");
  strcpy(sfButtons[1].value, "*");
  memset(sfButtons[0].value, 0, 10);
  memset(sfButtons[2].value, 0, 10);
  handleLoRa();
}

void handleSF10() {
  printf("handleSF10\n");
  mySF = 10;
  setSF();
  displayStatus((char *)"SF set to 10.");
  strcpy(sfButtons[0].value, "*");
  memset(sfButtons[1].value, 0, 10);
  memset(sfButtons[2].value, 0, 10);
  handleLoRa();
}

void handleSF12() {
  printf("handleSF12\n");
  mySF = 12;
  setSF();
  displayStatus((char *)"SF set to 12.");
  strcpy(sfButtons[2].value, "*");
  memset(sfButtons[0].value, 0, 10);
  memset(sfButtons[1].value, 0, 10);
  handleLoRa();
}

void setBW() {
  // set bandwidth
  if (radio.setBandwidth(myBW) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
    printf("Selected bandwidth is invalid for this module!\n");
  }
  char txt[32];
  sprintf(txt, (char *)"BW set to %.1f.", myBW);
  displayStatus(txt);
  sprintf(loRaButtons[1].value, "%.1f", myBW);
}

void handleBW() {
  printf("handleBW\n");
  clearArea(0, level2, epd_width(), 130);
  currButtons = bwButtons;
  btnCount = sizeof(bwButtons) / sizeof(kButton);
  int x = drawMenu(10, level2);
}

void handleBW62() {
  printf("handleBW62\n");
  myBW = 62.5;
  setBW();
  displayStatus((char *)"BW set to 62.5.");
  strcpy(bwButtons[0].value, "*");
  memset(bwButtons[1].value, 0, 10);
  memset(bwButtons[2].value, 0, 10);
  memset(bwButtons[3].value, 0, 10);
  handleLoRa();
}

void handleBW125() {
  printf("handleBW125\n");
  myBW = 125.0;
  setBW();
  strcpy(bwButtons[1].value, "*");
  memset(bwButtons[0].value, 0, 10);
  memset(bwButtons[2].value, 0, 10);
  memset(bwButtons[3].value, 0, 10);
  handleLoRa();
}

void handleBW250() {
  printf("handleBW250\n");
  myBW = 250.0;
  setBW();
  strcpy(bwButtons[2].value, "*");
  memset(bwButtons[0].value, 0, 10);
  memset(bwButtons[1].value, 0, 10);
  memset(bwButtons[3].value, 0, 10);
  handleLoRa();
}

void handleBW500() {
  printf("handleBW500\n");
  myBW = 500.0;
  setBW();
  strcpy(bwButtons[3].value, "*");
  memset(bwButtons[0].value, 0, 10);
  memset(bwButtons[1].value, 0, 10);
  memset(bwButtons[2].value, 0, 10);
  handleLoRa();
}

void setCR() {
  // set coding rate
  if (radio.setCodingRate(myCR) == RADIOLIB_ERR_INVALID_CODING_RATE) {
    printf("Selected coding rate is invalid for this module!\n");
  }
  char txt[32];
  sprintf(txt, (char *)"CR set to %d.", myCR);
  displayStatus(txt);
  sprintf(loRaButtons[3].value, "%d", myCR);
}

void handleCR() {
  printf("handleCR\n");
  clearArea(0, level2, epd_width(), 130);
  currButtons = crButtons;
  btnCount = sizeof(crButtons) / sizeof(kButton);
  int x = drawMenu(10, level2);
}

void handleCR5() {
  printf("handleCR5\n");
  myCR = 5;
  setCR();
  strcpy(crButtons[0].value, "*");
  memset(crButtons[1].value, 0, 10);
  memset(crButtons[2].value, 0, 10);
  memset(crButtons[3].value, 0, 10);
  handleLoRa();
}

void handleCR6() {
  printf("handleCR6\n");
  myCR = 6;
  setCR();
  strcpy(crButtons[1].value, "*");
  memset(crButtons[0].value, 0, 10);
  memset(crButtons[2].value, 0, 10);
  memset(crButtons[3].value, 0, 10);
  handleLoRa();
}

void handleCR7() {
  printf("handleCR7\n");
  myCR = 7;
  setCR();
  strcpy(crButtons[2].value, "*");
  memset(crButtons[0].value, 0, 10);
  memset(crButtons[1].value, 0, 10);
  memset(crButtons[3].value, 0, 10);
  handleLoRa();
}

void handleCR8() {
  printf("handleCR8\n");
  myCR = 8;
  setCR();
  strcpy(crButtons[3].value, "*");
  memset(crButtons[0].value, 0, 10);
  memset(crButtons[1].value, 0, 10);
  memset(crButtons[2].value, 0, 10);
  handleLoRa();
}

void handleFreq() {
  printf("handleFreq\n");
}

void handlePing() {
  if (myMode == loraMS) sendMSping();
  else sendPing();
}

void handleBL() {
  blON = 100 - blON;
  printf("Setting B/L to %d\n", blON);
  analogWrite(BOARD_BL_EN, blON);
}

void handleMsg() {
  displayingPackets = !displayingPackets;
  displayPackets();
}

int clickButtonCheck(int16_t x, int16_t y) {
  for (int ix = 0; ix < btnCount; ix++) {
    if (
      x >= currButtons[ix].x && y >= currButtons[ix].y && x < (currButtons[ix].x + btnWidth) && y < currButtons[ix].y + btnHeight)
      return ix;
  }
  return -1;
}

int drawMenu(int16_t fromPX, int16_t fromPY) {
  clearArea(0, fromPY, epd_width(), 130);
  printf("%d menus\n", btnCount);
  int16_t px = fromPX;
  int16_t py = fromPY;
  for (uint8_t ix = 0; ix < btnCount; ix++) {
    currButtons[ix].x = px;
    currButtons[ix].y = py;
    epd_poweron();
    drawButton(currButtons[ix], bgColor);
    epd_poweroff();
    px += btnWidth + 6;
    if (px > maxPX) {
      px = minPX;
      py = py + btnHeight + 10;
      printf("Resetting p: %d. Incrementing py: %d\n", px, py);
    }
  }
  // showBattery();
  epd_poweron();
  checkError(epd_hl_update_screen(&hl, MODE_GL16, temperature));
  epd_poweroff();
  return py + btnHeight + 10;
}

void drawButton(kButton B, uint8_t myColor) {
  EpdRect myRect = {
    .x = B.x,
    .y = B.y,
    .width = btnWidth,
    .height = btnHeight,
  };
  EpdRect myRect1 = {
    .x = B.x + 3,
    .y = B.y + 3,
    .width = btnWidth - 6,
    .height = btnHeight - 6,
  };
  printf("Drawing %s from %d,%d to %d, %d\n", B.label, B.x, B.y, (B.x + btnWidth - 1), (B.y + btnHeight - 1));
  epd_poweron();
  epd_fill_rect(myRect, 0, fb);
  epd_fill_rect(myRect1, myColor, fb);
  EpdFontProperties font_props = epd_font_properties_default();
  font_props.flags = EPD_DRAW_ALIGN_CENTER;
  int cx = B.x + myRect.width / 2;
  int cy = B.y + (myRect.width / 3 * 2) - 10;
  B.label[4] = '\0';
  epd_write_string(font20, B.label, &cx, &cy, fb, &font_props);
  cx = B.x + myRect.width / 2 + 1;
  cy = B.y + (myRect.width / 3 * 2) - 9;
  epd_write_string(font20, B.label, &cx, &cy, fb, &font_props);
  cx = B.x + myRect.width / 2 + 1;
  cy = B.y + myRect.height - 8;
  epd_write_string(font8b, B.value, &cx, &cy, fb, &font_props);
  epd_poweroff();
}

