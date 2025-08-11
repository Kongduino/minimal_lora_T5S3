using namespace std;
#pragma once
#include <vector>
#include <RadioLib.h>
#include "ExtensionIOXL9555.hpp"
#include "key.h"
#include "aes.c"

ExtensionIOXL9555 io;
// SX1262 has the following connections:
// NSS pin:   10
// DIO1 pin:  2
// NRST pin:  3
// BUSY pin:  9
SX1262 radio = new Module(LORA_CS, LORA_IRQ, LORA_RST, LORA_BUSY);
int transmissionState = RADIOLIB_ERR_NONE;
// flag to indicate that a packet was sent
volatile bool transmittedFlag = false;
// counter to keep track of transmitted packets
int pingCount = 0;
// flag to indicate that a packet was received
volatile bool receivedFlag = false;

enum fnMode {
  loraMS = 0,
  loraP2P = 1
};

enum fnMode myMode = loraMS;

// this function is called when a complete packet
// is transmitted by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void set_Tx_Flag(void) {
  // we sent a packet, set the flag
  transmittedFlag = true;
}

void set_Rx_Flag(void) {
  // we got a packet, set the flag
  receivedFlag = true;
}

void io_extend_lora_gps_power_on(bool en) {
  const uint8_t chip_address = XL9555_SLAVE_ADDRESS0;
  if (!io.init(Wire, BOARD_SDA, BOARD_SCL, chip_address)) {
    while (1) {
      printf("Failed to find XL9555 - check your wiring!\n");
      delay(1000);
    }
  }
  // Set PORT0 as input, mask = 0xFF = all pin input
  io.configPort(ExtensionIOXL9555::PORT0, 0x00);
  // Set PORT1 as input,mask = 0xFF = all pin input
  io.configPort(ExtensionIOXL9555::PORT1, 0xFF);
  printf("Power on LoRa and GPS!\n");
  if (en)
    io.digitalWrite(ExtensionIOXL9555::IO0, HIGH);
  else
    io.digitalWrite(ExtensionIOXL9555::IO0, LOW);
  delay(1500);
}

void loraSetup() {
  // lora and sd use the same spi, in order to avoid mutual influence;
  // before powering on, all CS signals should be pulled high and in an unselected state;
  pinMode(LORA_CS, OUTPUT);
  digitalWrite(LORA_CS, HIGH);
  // This must be turned on, otherwise LoRa and GPS will not work
  io_extend_lora_gps_power_on(true);
  SPI.begin(BOARD_SPI_SCLK, BOARD_SPI_MISO, BOARD_SPI_MOSI);
  // initialize SX1262 with default settings
  printf("[SX1262] Initializing...\n");
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    printf("success!\n");
  } else {
    printf("[SX1262] Init failed, code %d\n", state);
    while (true)
      ;
  }
  // set the function that will be called
  // when packet transmission is finished
  radio.setPacketSentAction(set_Tx_Flag);
  radio.setPacketReceivedAction(set_Rx_Flag);
  if (radio.setFrequency(myFreq / 1e6) == RADIOLIB_ERR_INVALID_FREQUENCY) {
    printf("Selected frequency is invalid for this module!\n");
    while (true)
      ;
  }
  // set bandwidth to 250 kHz
  if (radio.setBandwidth(myBW) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
    printf("Selected bandwidth is invalid for this module!\n");
    while (true)
      ;
  }
  // set spreading factor to 10
  if (radio.setSpreadingFactor(mySF) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
    printf("Selected spreading factor is invalid for this module!\n");
    while (true)
      ;
  }
  // set coding rate to 6
  if (radio.setCodingRate(myCR) == RADIOLIB_ERR_INVALID_CODING_RATE) {
    printf("Selected coding rate is invalid for this module!\n");
    while (true)
      ;
  }
  // set LoRa sync word to 0x2B
  if (radio.setSyncWord(0x2B) != RADIOLIB_ERR_NONE) {
    printf("Unable to set sync word!\n");
    while (true)
      ;
  }
  // set output power to 22 dBm (accepted range is -17 - 22 dBm)
  if (radio.setOutputPower(22) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
    printf("Selected output power is invalid for this module!\n");
    while (true)
      ;
  }
  // set over current protection limit to 80 mA (accepted range is 45 - 240 mA)
  // NOTE: set value to 0 to disable overcurrent protection
  if (radio.setCurrentLimit(140) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT) {
    printf("Selected current limit is invalid for this module!\n");
    while (true)
      ;
  }
  // set LoRa preamble length to 15 symbols (accepted range is 0 - 65535)
  if (radio.setPreambleLength(16) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH) {
    printf("Selected preamble length is invalid for this module!\n");
    while (true)
      ;
  }
  // disable CRC
  if (radio.setCRC(false) == RADIOLIB_ERR_INVALID_CRC_CONFIGURATION) {
    printf("Selected CRC is invalid for this module!\n");
    while (true)
      ;
  }
  // Some SX126x modules have TCXO (temperature compensated crystal
  // oscillator). To configure TCXO reference voltage,
  // the following method can be used.
  if (radio.setTCXO(1.8) == RADIOLIB_ERR_INVALID_TCXO_VOLTAGE) {
    printf("Selected TCXO voltage is invalid for this module!\n");
    while (true)
      ;
  }
  // Some SX126x modules use DIO2 as RF switch. To enable
  // this feature, the following method can be used.
  // NOTE: As long as DIO2 is configured to control RF switch,
  //       it can't be used as interrupt pin!
  if (radio.setDio2AsRfSwitch() != RADIOLIB_ERR_NONE) {
    printf("[SX1262] Failed to set DIO2 as RF switch!\n");
    while (true)
      ;
  }
  printf("All settings succesfully applied!\n");

  // start listening for LoRa packets
  Serial.print(F("[SX1262] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("[SX1262] Start Receive failed, code "));
    Serial.println(state);
    while (true)
      ;
  }
}

void showLoraSettings() {
  char text[32];
  epd_poweron();
  uint8_t yStep = 16;
  EpdRect myRect = {
    .x = 10,
    .y = 880,
    .width = 200,
    .height = 100,
  };
  epd_fill_rect(myRect, 0, fb);
  myRect.x += 2;
  myRect.y += 2;
  myRect.width -= 4;
  myRect.height -= 4;
  epd_fill_rect(myRect, 88, fb);
  EpdFontProperties font_props = epd_font_properties_default();
  font_props.flags = EPD_DRAW_ALIGN_LEFT;
  int px, cx;
  px = 14;
  cx = px;
  int py, cy;
  py = myRect.y + yStep;
  cy = py;
  sprintf(text, "Freq: %.4f MHz\n", (myFreq / 1e6));
  printf(text);
  epd_write_string(font8b, text, &cx, &cy, fb, &font_props);
  cx = px;
  py += yStep;
  cy = py;
  sprintf(text, "SF: %d\n", mySF);
  printf(text);
  epd_write_string(font8b, text, &cx, &cy, fb, &font_props);
  cx = px;
  py += yStep;
  cy = py;
  sprintf(text, "BW: %.1f KHz\n", myBW);
  printf(text);
  epd_write_string(font8b, text, &cx, &cy, fb, &font_props);
  cx = px;
  py += yStep;
  cy = py;
  sprintf(text, "CR: 4/%d\n", myCR);
  printf(text);
  epd_write_string(font8b, text, &cx, &cy, fb, &font_props);
  epd_poweroff();
}

void displayPackets() {
  Serial.println("displayPackets()!");
  char text[32];
  uint8_t yStep = 21;
  epd_poweron();
  EpdRect myRect = {
    .x = 10,
    .y = 680,
    .width = epd_width(),
    .height = 100,
  };
  if (displayingPackets == false) {
    epd_fill_rect(myRect, 255, fb);
    epd_poweroff();
    epd_poweron();
    checkError(epd_hl_update_screen(&hl, MODE_GL16, temperature));
    epd_poweroff();
    return;
  }
  uint16_t pks = myPackets.size();
  if (pks == 0) {
    displayingPackets = false;
    return;
  }
  epd_fill_rect(myRect, 0, fb);
  myRect.x += 2;
  myRect.y += 2;
  myRect.width -= 4;
  myRect.height -= 4;
  epd_fill_rect(myRect, 88, fb);
  EpdFontProperties font_props = epd_font_properties_default();
  font_props.flags = EPD_DRAW_ALIGN_LEFT;
  int px, cx;
  px = 22;
  cx = px;
  int py, cy;
  py = myRect.y + 33;
  cy = py;
//   EpdRect myRect = {
//     .x = 220,
//     .y = 560,
//     .width = 400,
//     .height = 418,
//   };
//   epd_fill_rect(myRect, 0, fb);
//   myRect.x += 2;
//   myRect.y += 2;
//   myRect.width -= 4;
//   myRect.height -= 4;
//   epd_fill_rect(myRect, 88, fb);
//   EpdFontProperties font_props = epd_font_properties_default();
//   font_props.flags = EPD_DRAW_ALIGN_LEFT;
//   int px, cx;
//   px = 224;
//   cx = px;
//   int py, cy;
//   py = myRect.y + yStep;
//   cy = py;
//   uint8_t start = 0;
//   if (pks > 6)
//     start = pks - 6;
//   for (uint8_t ix = start; ix < pks; ix++) {
    if(displayingPacketNum == -1) displayingPacketNum = 0;
    kPacket pk = myPackets[displayingPacketNum];
    sprintf(text, "%d:  %s --> %s", displayingPacketNum, pk.from, pk.to);
    Serial.println(text);
    epd_write_string(font, text, &cx, &cy, fb, &font_props);
    py += yStep;
    cy = py;
    cx = px;
    sprintf(text, " . RSSI: %.1f, SNR: %.1f", pk.RSSI, pk.SNR);
    Serial.println(text);
    epd_write_string(font, text, &cx, &cy, fb, &font_props);
    py += yStep;
    cy = py;
    cx = px;
    sprintf(text, " . msg ID: %s", pk.msgID);
    Serial.println(text);
    epd_write_string(font, text, &cx, &cy, fb, &font_props);
//     py += yStep;
//     cy = py;
//     cx = px;
//   }
  epd_poweroff();
  epd_poweron();
  checkError(epd_hl_update_screen(&hl, MODE_GL16, temperature));
  epd_poweroff();
}

void addPacket(char *raw, int numBytes, float RSSI, float SNR) {
  kPacket pk;
  pk.SNR = SNR;
  pk.RSSI = RSSI;
  sprintf(pk.from, "!%02x%02x%02x%02x", raw[7], raw[6], raw[5], raw[4]);
  sprintf(pk.to, "!%02x%02x%02x%02x", raw[3], raw[2], raw[1], raw[0]);
  sprintf(pk.msgID, "!%02x%02x%02x%02x", raw[11], raw[10], raw[9], raw[8]);
  memset(pk.nonce, 0, 16);
  memcpy(pk.nonce, raw + 8, 4);      // msgID
  memcpy(pk.nonce + 8, raw + 4, 4);  // from/sender
  /*
    https://gitlab.com/crankylinuxuser/meshtastic_sdr/-/blob/master/python%20scripts/meshtastic_gnuradio_RX.py?ref_type=heads
    # Build the nonce. This is (packetID)+(00000000)+(sender)+(00000000) for a total of 128bit
    # Even though sender is a 32 bit number, internally its used as a 64 bit number.
    # Needs to be a bytes array for AES function.

    aesNonce = meshPacketHex['packetID'] + b'\x00\x00\x00\x00' + meshPacketHex['sender'] + b'\x00\x00\x00\x00'
  */
  myPackets.push_back(pk);
//   if (numBytes > 32) {
//     // no point if the packet is smaller than that
//     AES_ctx *ctx;
//     AES_init_ctx_iv(ctx, aesKey, pk.nonce);
//     int dataLen = numBytes - 32;
//     unsigned char data[256];
//     memcpy(data, raw + 32, dataLen);
//     uint8_t difference = dataLen % 16;
//     if(difference > 0) {
//       memset(data + dataLen, difference, difference);
//       dataLen += (16 - difference);
//     }
//     printf("* Final dataLen: %d\n", dataLen); delay(10);
//     printf("* data [%d bytes] before AES_CTR_xcrypt_buffer.\n", (numBytes - 32)); delay(10);
//     hexDump(data, dataLen);
//     AES_CTR_xcrypt_buffer(ctx, data, dataLen);
//     printf("* data [%d bytes] after AES_CTR_xcrypt_buffer.\n", (numBytes - 32)); delay(10);
//     hexDump(data, dataLen);
//   }
  while (myPackets.size() > 100)
    myPackets.erase(myPackets.begin(), myPackets.begin() + 1);
//  displayPackets();
}

/*
   +------------------------------------------------+ +----------------+
   |.0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .a .b .c .d .e .f | |      ASCII     |
   +------------------------------------------------+ +----------------+
 0.|ff ff ff ff f4 0e 0a a2 a5 d2 74 da 63 6e 00 f4 | |tcn.............|
 1.|67 54 8f d7 db fa ed de a1 f9 b0 1b d2 0e       | |gT..............|
   +------------------------------------------------+ +----------------+
DECAFBAD sent on LongMod main channel

*/

char msPingPacket[] = {
  0xff, 0xff, 0xff, 0xff, 0xad, 0xfb, 0xca, 0xde, 0xa5, 0xd2, 0x74, 0xda, 0x63, 0x6e, 0x00,
  0xf4, 0x67, 0x54, 0x8f, 0xd7, 0xdb, 0xfa, 0xed, 0xde, 0xa1, 0xf9, 0xb0, 0x1b, 0xd2, 0x0e
};

void sendPing() {
  printf("[SX1262] Sending a PING packet...\n");
  char txt[16];
  sprintf(txt, "PING #%d", pingCount++);
  displayStatus(txt);
  transmissionState = radio.startTransmit(txt);
}

void sendMSping() {
  printf("[SX1262] Sending a Meshtastic PING packet...\n");
  char txt[32];
  sprintf(txt, "Meshtastic PING #%d", pingCount++);
  displayStatus(txt);
  transmissionState = radio.startTransmit(msPingPacket);
}
