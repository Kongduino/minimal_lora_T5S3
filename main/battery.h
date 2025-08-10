#pragma once
#include "bq27220.h"
#include "esp_timer.h"

BQ27220 bq;
BQ27220BatteryStatus batt;
uint32_t lastBatteryUpdate = millis();
void showBattery(void *arg);
void initBaterrySchedule();

void initBaterrySchedule() {
  const esp_timer_create_args_t periodic_timer_args = {
    .callback = &showBattery,
    /* argument passed to the callback */
    .arg = NULL,
    /* name is optional, but a good practice.
     * esp_timer_dump(stdout) will print this */
    .name = "battery_periodic_timer"
  };
  esp_timer_handle_t battery_periodic_timer;
  ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &battery_periodic_timer));
  // Start the timer
  /* Define the period (in microseconds). */
  int64_t period_us = 30000000; // 5 seconds
  ESP_ERROR_CHECK(esp_timer_start_periodic(battery_periodic_timer, period_us));
  printf("Battery task inited at interval of %.1f secs.\n", (period_us/1e6));
}

void showBattery(void *arg) {
  bq.getBatteryStatus(&batt);
  uint16_t voltage = bq.getVoltage();
  printf("\n\nStatus = %x\n", batt.full);
  printf("Status = %s\n", bq.getIsCharging() ? "Charging" : "Discharging");
  printf("Voltage = %.2f V\n", (voltage / 1000.0));
  printf("Current = %d mA\n", bq.getCurrent());
  printf("Temperature = %.2f C\n", (float)(bq.getTemperature() / 10.0 - 273.15));
  printf("full capacity = %d mAh\n", bq.getFullChargeCapacity());
  printf("Remaining capacity = %d mAh\n", bq.getRemainingCapacity());
  float percent = bq.getStateOfCharge();
  printf("state of charge = %d\n", bq.getStateOfCharge());
  printf("state of health = %d\n\n", bq.getStateOfHealth());
  int cx = 436;
  int cy = 20;
  EpdRect area = {
    .x = cx,
    .y = cy,
    .width = 86,
    .height = 40
  };
  clearArea(cx, cy, area.width, area.height);
  cx = 460;
  area.x = cx;
  area.width = 56;
  area.height = 26;
  epd_poweron();
  epd_fill_rect(area, 0, fb);
  area.y += 2;
  area.x += 2;
  area.width -= 4;
  area.height -= 4;
  epd_fill_rect(area, 255, fb);
  area.x = cx + 3;
  area.y = cy + 3;
  area.height -= 2;
  area.width = (int)(percent / 2); // % * 50
  printf(
    "Black out: %d,%d to %d,%d\n",
    area.x, area.y,
    (area.x + area.width), (area.y + area.height)
  );
  epd_fill_rect(area, 0, fb);
  // + nib
  area.x = 516;
  area.y += 3;
  area.width = 3;
  area.height -= 6;
  epd_fill_rect(area, 0, fb);
  if (!bq.getIsCharging()) {
    printf("Not charging...\n");
    area.x = 430;
    area.y = 20;
    area.width = 30;
    area.height = 26;
    clearArea(area.x, area.y, area.width, area.height);
  } else {
    printf("Charging...\n");
    uint16_t px, py;
    px = 443;
    py = 20;
    for (int16_t ix = 0; ix < 10; ix++) {
      epd_draw_line(px + 4, py, px + ix - 4, py + 14, 0, fb);
    }
    py += 12;
    for (int16_t ix = 2; ix < 12; ix++) {
      epd_draw_line(px + ix, py, px, py + 14, 0, fb);
    }
  }
  EpdFontProperties fp = epd_font_properties_default();
  fp.flags = EPD_DRAW_ALIGN_CENTER;
  char text[8];
  sprintf(text, "%d%%", (int)percent);
  cx = 487; cy = 60;
  epd_write_string(font8b, text, &cx, &cy, fb, &fp);
  epd_poweroff();
  displayLogo();
}

