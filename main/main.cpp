#include <Wire.h>
#include <SPI.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <epdiy.h>
#include "sdkconfig.h"
#include "firasans_12.h"
#include "firasans_20.h"
#include "opensans8.h"
#include "opensans8b.h"
#include <Arduino.h>
#include "TouchDrvGT911.hpp"
#define XPOWERS_CHIP_BQ25896
#include <XPowersLib.h>
#include "utilities.h"
#include "battery.h"
#include "lora.h"
#include "hexdump.h"
#include "menus.h"
#include "gnss.h"
#include "ExtensionIOXL9555.hpp"

using namespace std;

TouchDrvGT911 touch;
void idf_setup();
void idf_loop();

#define WAVEFORM EPD_BUILTIN_WAVEFORM

// choose the default demo board depending on the architecture
#ifdef CONFIG_IDF_TARGET_ESP32S3
#define DEMO_BOARD epd_board_v7
#endif

void displayStatus(char *txt) {
  epd_poweron();
  clearArea(0, baseLine, epd_width(), 33);
  EpdFontProperties font_props = epd_font_properties_default();
  font_props.flags = EPD_DRAW_ALIGN_LEFT;
  int cx = 10, cy = baseLine + 30;
  epd_write_string(font20, txt, &cx, &cy, fb, &font_props);
  epd_poweroff();
  epd_poweron();
  checkError(epd_hl_update_screen(&hl, MODE_GL16, temperature));
  epd_poweroff();
}

void checkError(enum EpdDrawError err) {
  if (err != EPD_DRAW_SUCCESS) {
    ESP_LOGE("demo", "draw error: %X", err);
  }
}

void idf_setup() {
  Serial.begin(115200);
  Wire.begin(39, 40);
  loraSetup();
  gps_init();
  touch.setPins(SENSOR_RST, SENSOR_IRQ);
  if (!touch.begin(Wire, GT911_SLAVE_ADDRESS_L, SENSOR_SDA, SENSOR_SCL)) {
    printf("Failed to find GT911 - check your wiring!\n");
    while (1) {
      delay(1000);
    }
  }
  printf("%s sensor init success!\n", "GT911");
  bq.init();

  // Set the center button to trigger the callback , Only for specific devices, e.g LilyGo-EPD47 S3 GT911
  touch.setHomeButtonCallback(mainMenu, NULL);
  touch.setInterruptMode(LOW_LEVEL_QUERY);
  epd_init(&DEMO_BOARD, &ED047TC1, EPD_LUT_64K);
  // Set VCOM for boards that allow to set this in software (in mV).
  // This will print an error if unsupported. In this case,
  // set VCOM using the hardware potentiometer and delete this line.
  epd_set_vcom(1560);
  hl = epd_hl_init(WAVEFORM);
  // Default orientation is EPD_ROT_LANDSCAPE
  epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);
  printf(
    "Dimensions after rotation, width: %d height: %d\n\n", epd_rotated_display_width(),
    epd_rotated_display_height());
  // The display bus settings for V7 may be conservative, you can manually
  // override the bus speed to tune for speed, i.e., if you set the PSRAM speed
  // to 120 MHz.
  // epd_set_lcd_pixel_clock_MHz(17);
  heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
  heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
  // select the font based on display width
  if (epd_width() < 1000) {
    font = &FiraSans_12;
  } else {
    font = &FiraSans_20;
  }
  char text[32];
  epd_poweron();
  epd_clear();
  temperature = epd_ambient_temperature();
  printf("current temperature: %d\n", temperature);
  displayLogo();
  EpdFontProperties fp = epd_font_properties_default();
  sprintf(text, "Minimal LoRa");
  int cx = 110, cy = 60;
  epd_write_string(font20, text, &cx, &cy, fb, &fp);
  cx = epd_width() / 2 + 40;
  cy = 100;
  fp.flags = EPD_DRAW_ALIGN_RIGHT;
  sprintf(text, "by Kongduino");
  cx = epd_width() / 2 + 40;
  cy = 100;
  epd_write_string(font8b, text, &cx, &cy, fb, &fp);
  epd_poweroff();
  mainMenu(NULL);
  // showBattery();
  epd_poweron();
  checkError(epd_hl_update_screen(&hl, MODE_GL16, temperature));
  epd_poweroff();
  // transmissionState = radio.startTransmit("Hello World!");
  // xTaskCreate(btn_task, "lora_task", 1024 * 3, NULL, INFARED_PRIORITY, &btn_handle);
  initBaterrySchedule();
}

int lastRslt = -1;

void idf_loop() {
  uint32_t now = millis();
  if ((now - lastClick) > 1200) lastRslt = -1;
  if (millis() > lastGPScheck + 30000) {
    printf("Checking GPS...\n");
    checkGNSS();
    lastGPScheck = millis();
    printf("done!\n");
    void showGPSinfo();
  }
  // check if the flag is set
  if (receivedFlag) {
    printf("receivedFlag: True\n");
    // reset flag
    receivedFlag = false;
    byte byteArr[256];
    int numBytes = radio.getPacketLength();
    uint16_t state = radio.readData(byteArr, numBytes);
    if (state == RADIOLIB_ERR_NONE && numBytes > 0) {
      if (numBytes == 0) {
        printf("Empty packet...\n");
      } else {
        // packet was successfully received
        printf("[SX1262] Received packet. Len: %d\n", numBytes);
        // print data of the packet
        hexDump(byteArr, numBytes);
        // print RSSI (Received Signal Strength Indicator)
        uint32_t packetStatus = radio.getPacketStatus();
        uint8_t rssiPkt = packetStatus & 0xFF;
        float RSSI = (-1.0 * rssiPkt / 2.0), SNR;
        uint8_t snrPkt = (packetStatus >> 8) & 0xFF;
        if (snrPkt < 128) {
          SNR = snrPkt / 4.0;
        } else {
          SNR = (snrPkt - 256) / 4.0;
        }
        printf("[SX1262] RSSI:\t\t%.2f dBm\n", RSSI);
        // print SNR (Signal-to-Noise Ratio)
        printf("[SX1262] SNR:\t\t%.2f dB\n", SNR);
        // print frequency error
        printf("[SX1262] Frequency error:\t%f Hz\n", radio.getFrequencyError());
        printf("Adding packet\n");
        addPacket((char*)byteArr, numBytes, RSSI, SNR);
      }
    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      // packet was received, but is malformed
      printf("CRC error: %d!", state);
    } else {
      // some other error occurred
      printf("[SX1262] Misc error, code %d\n", state);
    }
    state = radio.standby();
    delay(100);
    state = radio.startReceive();
  }
  // check if the previous transmission finished
  if (transmittedFlag) {
    // reset flag
    transmittedFlag = false;
    if (transmissionState == RADIOLIB_ERR_NONE) {
      // packet was successfully sent
      printf("transmission finished!\n");
      // NOTE: when using interrupt-driven transmit method,
      //       it is not possible to automatically measure
      //       transmission data rate using getDataRate()
    } else {
      printf("[SX1262] Transmission failed, code %d\n", transmissionState);
    }
    // clean up after transmission is finished
    // this will ensure transmitter is disabled,
    // RF switch is powered down etc.
    radio.finishTransmit();
    int state = radio.standby();
    delay(100);
    state = radio.startReceive();
  }
  if (touch.isPressed()) {
    uint8_t touched = touch.getPoint(x, y, touch.getSupportTouchPoint());
    if (touched > 0) {
      uint8_t i = touched - 1;
      printf("X[%d]: %d Y[%d]: %d\n", i, x[i], i, y[i]);
      delay(500);
      int rslt = clickButtonCheck(x[i], y[i]);
      if (rslt > -1 && rslt != lastRslt) {
        char txt[32];
        sprintf(txt, "You clicked btn %d\n", rslt);
        printf(txt);
        epd_poweron();
        drawButton(currButtons[rslt], bgColorI);
        epd_poweroff();
        epd_poweron();
        checkError(epd_hl_update_screen(&hl, MODE_GL16, temperature));
        epd_poweroff();
        epd_poweron();
        drawButton(currButtons[rslt], bgColor);
        epd_poweroff();
        epd_poweron();
        checkError(epd_hl_update_screen(&hl, MODE_GL16, temperature));
        epd_poweroff();
        lastClick = millis();
        lastClickedBtn = rslt;
        currButtons[rslt].ptr();
        lastRslt = rslt;
      }
    }
  }
  delay(10);
}
