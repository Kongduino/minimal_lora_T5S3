//#include "satellite_s.h"
#pragma once
using namespace std;
#include <string>
#include <vector>

int drawMenu(int16_t, int16_t);
void handleSF();
void handleSF11();
void handleSF10();
void handleSF12();
void handleBW();
void handleBW62();
void handleBW125();
void handleBW250();
void handleBW500();
void handleFreq();
void handlePing();
void handleLoRa();
void handleMain();
void handleOff();
void handleMode();
void handleCR();
void handleCR5();
void handleCR6();
void handleCR7();
void handleCR8();
void clearArea(int16_t, int16_t, int16_t, int16_t);
void displayStatus(char *);
void showLoraSettings();
void io_extend_lora_gps_power_on(bool);
void set_Tx_Flag(void);
void set_Rx_Flag(void);
void displayLogo();
void loraSetup();
void addPacket(char *, int, float, float);
void displayPackets();
void sendPing();
void sendMSping();
void hexDump(unsigned char *, int);
void showGPSinfo();
// void btn_task(void *);
void handleBL();
void handleMsg();
void handleModeMS();
void handleModeP2P();

// GNSS
void parseGPRMC(vector<string>);
void parseGPGGA(vector<string>);
void parseGPZDA(vector<string>);
void parseGPGLL(vector<string>);
void parseGPGSV(vector<string>);
void parseGPTXT(vector<string>);
void parseGPVTG(vector<string>);
void parseGPGSA(vector<string>);

#define BOARD_SCL (40)
#define BOARD_SDA (39)

#define BOARD_SPI_MISO (21)
#define BOARD_SPI_MOSI (13)
#define BOARD_SPI_SCLK (14)

#define TOUCH_SCL (BOARD_SCL)
#define TOUCH_SDA (BOARD_SDA)
#define TOUCH_INT (3)
#define TOUCH_RST (9)

#define RTC_SCL (BOARD_SCL)
#define RTC_SDA (BOARD_SDA)
#define RTC_IRQ (2)

#define SD_MISO (BOARD_SPI_MISO)
#define SD_MOSI (BOARD_SPI_MOSI)
#define SD_SCLK (BOARD_SPI_SCLK)
#define SD_CS (12)

#define LORA_MISO (BOARD_SPI_MISO)
#define LORA_MOSI (BOARD_SPI_MOSI)
#define LORA_SCLK (BOARD_SPI_SCLK)
#define LORA_CS (46)
#define LORA_IRQ (10)
#define LORA_RST (1)
#define LORA_BUSY (47)

#define BL_EN (11)

#define BOOT_BTN (0)

#ifndef CONFIG_PMU_SDA
#define CONFIG_PMU_SDA 39
#endif

#ifndef CONFIG_PMU_SCL
#define CONFIG_PMU_SCL 40
#endif

#ifndef CONFIG_PMU_IRQ
#define CONFIG_PMU_IRQ -1
#endif

const uint8_t i2c_sda = CONFIG_PMU_SDA;
const uint8_t i2c_scl = CONFIG_PMU_SCL;
const uint8_t pmu_irq_pin = CONFIG_PMU_IRQ;

#define SENSOR_SDA 39
#define SENSOR_SCL 40
#define SENSOR_IRQ 3
#define SENSOR_RST 9

#define BOARD_GPS_RXD 44
#define BOARD_GPS_TXD 43
#define BOARD_GPS_PPS 1
#define SerialMon Serial
#define SerialGPS Serial1

EpdiyHighlevelState hl;
const EpdFont *font;
const EpdFont *font20 = &FiraSans_20;
const EpdFont *font8 = &OpenSans8;
const EpdFont *font8b = &OpenSans8B;
int16_t x[5], y[5], baseLine;
uint8_t btnCount = 0, bgColor = 88, bgColorI = 6;
int minPX = 10, maxPX = 500, minPY = 110, btnWidth = 98, btnHeight = 96;
uint8_t *fb;
int temperature;
uint8_t mySF = 11, myCR = 8;
float myBW = 125.0;
uint32_t myFreq = 902687500;
uint32_t lastClick = millis();
uint8_t lastClickedBtn = 255;

typedef struct {
  char from[10];
  char to[10];
  char msgID[10];
  unsigned char nonce[16];
  float RSSI;
  float SNR;
} kPacket;
vector<kPacket> myPackets;

uint8_t buffer[256];
bool gps_init();
bool setupGPS();
static bool GPS_Recovery();
static int getAck(uint8_t *, uint16_t, uint8_t, uint8_t);
uint32_t lastGPScheck = millis() - 30000;
char myDate[32];
void checkError(enum EpdDrawError);
void displayLogo() {
  uint16_t px = 30, py = 28;
  fb = epd_hl_get_framebuffer(&hl);
  uint8_t incr = 15;
  uint8_t colours[] = {
    32, 50, 32, 50, 32, 50
  };
  for (uint8_t ix = 0; ix < 6; ix++) {
    epd_fill_circle(px, py, 15, colours[ix], fb);
    px += 28;
    if (px > 90) {
      py += 28;
      px = 30 + incr;
      incr += 15;
    }
  }
}

void clearArea(int16_t x, int16_t y, int16_t width, int16_t height) {
  EpdRect area = {
    .x = x,
    .y = y,
    .width = width,
    .height = height
  };
  epd_fill_rect(area, 255, fb);
}

#define BOARD_BL_EN (11)
#define BOARD_PCA9535_INT (38)
#define BOARD_BOOT_BTN (0)
#define GPS_PRIORITY (configMAX_PRIORITIES - 1)
#define LORA_PRIORITY (configMAX_PRIORITIES - 2)
#define WS2812_PRIORITY (configMAX_PRIORITIES - 3)
#define BATTERY_PRIORITY (configMAX_PRIORITIES - 4)
#define INFARED_PRIORITY (configMAX_PRIORITIES - 5)
TaskHandle_t btn_handle;
uint8_t blON = 0;
int8_t displayingPacketNum = -1;
bool displayingPackets = false;

uint8_t button_read(void) {
  uint8_t io_val0 = pca9555_read_input(0, 0);
  uint8_t io_val = pca9555_read_input(0, 1);
  printf("io_extend : 0x%x\n", io_val);
  return io_val; //!(io_val & (PCA_PIN_PC12 >> 8));
}

// void btn_task(void *param) {
//   while (1) {
//     if (digitalRead(BOARD_PCA9535_INT) == LOW) {
//       uint32_t startMillis = millis();
//       uint8_t io_val = button_read();
//       if (io_val == 0xc0) {
//         delay(500);
//         Serial.print("\nIO48 button Press\n");
//       } else {
//         uint32_t endMillis = millis() - startMillis;
//         printf("Time elapsed: %d\n", endMillis);
//         Serial.print("release: displayPackets\n");
//         displayingPacketNum += 1;
//         displayingPackets = true;
//         uint16_t pks = myPackets.size();
//         if (displayingPacketNum == pks) {
//           displayingPacketNum = -1;
//           analogWrite(BOARD_BL_EN, 0);
//           displayingPackets = false;
//         } else if (displayingPacketNum == 0) {
//           analogWrite(BOARD_BL_EN, 100);
//         }
//         displayPackets();
//       }
//     } else if (digitalRead(BOARD_BOOT_BTN) == LOW) {
//       uint8_t io_val = button_read();
//       if (io_val == 0xc4) {
//         delay(500);
//         printf("\nBoot button Press. --> ");
//       } else {
//         printf("io_extend end: displayPackets. blON = %d\n\n", blON);
//         blON = 100 - blON;
//         analogWrite(BOARD_BL_EN, blON);
//         displayingPacketNum = -1;
//         displayingPackets = false;
//         displayPackets();
//       }
//     }
//     delay(300);
//   }
// }
