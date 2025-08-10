using namespace std;
#include <string>
#include <vector>
#include "myLocation.h"

uint8_t ix = 0;
vector<string> userStrings;
char dateUTC[32] = { 0 };
char timeUTC[32] = { 0 };
uint8_t SIV = 0;
float latitude, longitude;
bool validCoords = false, validDate = false, validTime = false;

float toRad(float x) {
  return x * 3.141592653 / 180;
}

float haversine(float lat1, float lon1, float lat2, float lon2) {
  float R = 6371;  // km
  float x1 = lat2 - lat1;
  float dLat = toRad(x1);
  float x2 = lon2 - lon1;
  float dLon = toRad(x2);
  float a = sin(dLat / 2) * sin(dLat / 2) + cos(toRad(lat1)) * cos(toRad(lat2)) * sin(dLon / 2) * sin(dLon / 2);
  float c = 2 * atan2(sqrt(a), sqrt(1 - a));
  float d = R * c;
  return round(d * 100.0) / 100;
}

float parseDegrees(const char *term) {
  float value = (float)(atof(term) / 100.0);
  uint16_t left = (uint16_t)value;
  value = (value - left) * 1.66666666666666;
  value += left;
  return value;
}

vector<string> parseNMEA(string nmea) {
  vector<string> result;
  if (nmea.at(0) != '$') {
    Serial.println("Not an NMEA sentence!");
    return result;
  }
  size_t lastFound = 0;
  size_t found = nmea.find(",", lastFound);
  while (found < nmea.size() && found != string::npos) {
    string token = nmea.substr(lastFound, found - lastFound);
    result.push_back(token);
    lastFound = found + 1;
    found = nmea.find(",", lastFound);
  }
  string token = nmea.substr(lastFound, found - lastFound);
  result.push_back(token);
  lastFound = found + 1;
  found = nmea.find(",", lastFound);
  return result;
}

void parseGPRMC(vector<string> result) {
  if (result.at(1) != "") {
    sprintf(timeUTC, "Time: %s:%s:%s UTC", result.at(1).substr(0, 2).c_str(), result.at(1).substr(2, 2).c_str(), result.at(1).substr(4, 2).c_str());
    Serial.println((const char*)timeUTC);
    validTime = true;
  } else validTime = false;
  // if (result.at(2) == "V") Serial.println("Invalid fix!");
  // else Serial.println("Valid fix!");
  if (result.at(2) == "A") {
    float newLatitude, newLongitude;
    newLatitude = parseDegrees(result.at(3).c_str());
    newLongitude = parseDegrees(result.at(5).c_str());
    validCoords = true;
    float distance = haversine(latitude, longitude, newLatitude, newLongitude);
    if (distance > 20.0) {
      latitude = newLatitude;
      longitude = newLongitude;
      sprintf((char*)buffer, "[%s] Coordinates: %3.8f %c, %3.8f %c\n", result.at(0).c_str(), latitude, result.at(4).c_str()[0], longitude, result.at(6).c_str()[0]);
      Serial.print((const char*)buffer);
    }
  } else if (validCoords) validCoords = false;
}

void parseGPGGA(vector<string> result) {
  if (result.at(1) != "") {
    sprintf(timeUTC, "Time: %s:%s:%s UTC", result.at(1).substr(0, 2).c_str(), result.at(1).substr(2, 2).c_str(), result.at(1).substr(4, 2).c_str());
    Serial.println((const char*)timeUTC);
    validTime = true;
  } else validTime = false;
  //  if (result.at(6) == "0") Serial.println("Invalid fix!");
  //  else Serial.println("Valid fix!");
  if (result.at(2) != "") {
    latitude = parseDegrees(result.at(2).c_str());
    longitude = parseDegrees(result.at(4).c_str());
    sprintf((char*)buffer, "[%s] Coordinates: %3.8f %c, %3.8f %c\n", result.at(0).c_str(), latitude, result.at(3).c_str()[0], longitude, result.at(5).c_str()[0]);
    Serial.print((const char*)buffer);
    if (result.at(3).c_str() == "S") latitude = latitude * -1;
    if (result.at(5).c_str() == "W") longitude = longitude * -1;
    validCoords = true;
  } else if (validCoords) validCoords = false;
  if (result.at(7) != "") {
    sprintf((char*)buffer, "[GGA/ANT] SIV: %s\n", result.at(7).c_str());
      Serial.print((const char*)buffer);
      SIV = atoi(result.at(7).c_str());
  }
}

void parseGPDHV(vector<string> result) {
  if (result.at(1) != "") {
    sprintf(timeUTC, "Time: %s:%s:%s UTC", result.at(1).substr(0, 2).c_str(), result.at(1).substr(2, 2).c_str(), result.at(1).substr(4, 2).c_str());
    Serial.println((const char*)timeUTC);
    validTime = true;
  } else validTime = false;
  if (result.at(2) != "") {
    printf("3D Speed: %d mph", result.at(2));
  }
  if (result.at(3) != "") {
    printf("X Speed: %d mph", result.at(3));
  }
  if (result.at(4) != "") {
    printf("Y Speed: %d mph", result.at(4));
  }
  if (result.at(5) != "") {
    printf("Z Speed: %d mph", result.at(5));
  }
  if (result.at(6) != "") {
    printf("Hz Speed: %d mph", result.at(6));
  }
}

void parseGPZDA(vector<string> result) {
  if (result.at(1) != "") {
    sprintf(timeUTC, "Time: %s:%s:%s UTC", result.at(1).substr(0, 2).c_str(), result.at(1).substr(2, 2).c_str(), result.at(1).substr(4, 2).c_str());
    Serial.println((const char*)buffer);
    validTime = true;
  } else validTime = false;
  if (result.at(2) != "" && result.size() > 6) {
    sprintf(dateUTC, "Date: %c%c%c%c/%c%c/%c%c UTC",
      result.at(4)[0], result.at(4)[1], result.at(4)[2], result.at(4)[3],
      result.at(3)[0], result.at(3)[1],
      result.at(2)[0], result.at(2)[1]
    );
    Serial.println((const char*)dateUTC);
    validDate = true;
  } else validDate = false;
}

void parseGPGLL(vector<string> result) {
  if (result.at(1) != "") {
    latitude = parseDegrees(result.at(1).c_str());
    longitude = parseDegrees(result.at(3).c_str());
    sprintf((char*)buffer, "[%s] Coordinates: %3.8f %c, %3.8f %c\n", result.at(0).c_str(), latitude, result.at(2).c_str()[0], longitude, result.at(4).c_str()[0]);
    Serial.print((const char*)buffer);
    validCoords = true;
  } else if (validCoords) validCoords = false;
}

void parseGPGSV(vector<string> result) {
  if (result.at(1) != "") {
    uint8_t newSIV = atoi(result.at(3).c_str());
    // if (SIV != newSIV) {
      sprintf((char*)buffer, "[%s] Message %s / %s. SIV: %s\n", result.at(0).c_str(), result.at(2).c_str(), result.at(1).c_str(), result.at(3).c_str());
      Serial.print((const char*)buffer);
      SIV = newSIV;
    // }
  }
}

void parseGPTXT(vector<string> result) {
  //$GPTXT, 01, 01, 02, ANTSTATUS = INIT
  if (result.at(1) != "") {
    sprintf(
      (char*)buffer, " . Message %s / %s. Severity: %s\n . Message text: %s\n",
      result.at(2).c_str(), result.at(1).c_str(), result.at(3).c_str(), result.at(4).c_str()
    );
    Serial.print((const char*)buffer);
    displayStatus((char*)result.at(4).c_str());
  }
}

void parseGPVTG(vector<string> result) {
  Serial.println("Track Made Good and Ground Speed.");
  if (result.at(1) != "") {
    sprintf((char*)buffer, " . True track made good %s [%s].\n", result.at(1).c_str(), result.at(2).c_str());
    Serial.print((const char*)buffer);
  }
  if (result.at(3) != "") {
    sprintf((char*)buffer, " . Magnetic track made good %s [%s].\n", result.at(3).c_str(), result.at(4).c_str());
    Serial.print((const char*)buffer);
  }
  if (result.at(5) != "") {
    sprintf((char*)buffer, " . Speed: %s %s.\n", result.at(5).c_str(), result.at(6).c_str());
    Serial.print((const char*)buffer);
  }
  if (result.at(7) != "") {
    sprintf((char*)buffer, " . Speed: %s %s.\n", result.at(7).c_str(), result.at(8).c_str());
    Serial.print((char*)buffer);
  }
}

void parseGPGSA(vector<string> result) {
  // $GPGSA,A,3,15,29,23,,,,,,,,,,12.56,11.96,3.81
  Serial.println("GPS DOP and active satellites");
  if (result.at(1) == "A") Serial.println(" . Mode: Automatic");
  else if (result.at(1) == "M") Serial.println(" . Mode: Manual");
  else Serial.println(" . Mode: ???");
  if (result.at(2) == "1") {
    Serial.println(" . Fix not available.");
    return;
  } else if (result.at(2) == "2") Serial.println(" . Fix: 2D");
  else if (result.at(2) == "3") Serial.println(" . Fix: 3D");
  else {
    Serial.println(" . Fix: ???");
    return;
  }
  Serial.print(" . PDOP: ");
  Serial.println(result.at(result.size() - 3).c_str());
  Serial.print(" . HDOP: ");
  Serial.println(result.at(result.size() - 2).c_str());
  Serial.print(" . VDOP: ");
  Serial.println(result.at(result.size() - 1).c_str());
}

bool waitForDollar = true;

void checkGNSS() {
  while (SerialGPS.available()) {
    char c = SerialGPS.read();
    if (waitForDollar && c == '$') {
      waitForDollar = false;
    } else if (waitForDollar == false) {
      string dollar("$G");
      String XX = SerialGPS.readStringUntil('\r');
      printf(" * Received: $G%s\n", XX.c_str());
      string nextLine(XX.c_str());
      userStrings.push_back(dollar + nextLine.substr(0, nextLine.length() - 3));
      waitForDollar = true;
    }
  }
  printf("Number of lines: %d\n", userStrings.size());
  while (userStrings.size() > 0) {
    string nextLine = userStrings[0];
    printf(" * Processing: `%s`\n", nextLine.c_str());
    delay(50);
    userStrings.erase(userStrings.begin());
    if (nextLine.substr(0, 1) != "$") {
      Serial.print("Not an NMEA string!\n");
      // Serial.println(nextLine.c_str());
    } else {
      vector<string> result = parseNMEA(nextLine);
      int rs = result.size();
      printf("result has %d element%s\n", rs, rs < 2 ? "." : "s.");
      if (rs == 0) return;
      for(int ix = 0; ix < rs; ix++)
        printf(" * chunk %d: %s\n", ix, result.at(ix).c_str());
      string verb = result.at(0);
      if (verb.substr(3, 3) == "RMC") {
        parseGPRMC(result);
      } else if (verb.substr(3, 3) == "GSV") {
        parseGPGSV(result);
      } else if (verb.substr(3, 3) == "GGA" || verb.substr(3, 3) == "ANT") {
        parseGPGGA(result);
      } else if (verb.substr(3, 3) == "GLL") {
        parseGPGLL(result);
      } else if (verb.substr(3, 3) == "GSA") {
        //parseGPGSA(result);
      } else if (verb.substr(3, 3) == "VTG") {
        //parseGPVTG(result);
      } else if (verb.substr(3, 3) == "ZDA") {
        parseGPZDA(result);
      } else if (verb.substr(3, 3) == "DHV") {
        parseGPDHV(result);
      } else if (verb.substr(3, 3) == "TXT") {
        parseGPTXT(result);
      }
    }
  }
  showGPSinfo();
}


bool setupGPS() {
  // L76K GPS USE 9600 BAUDRATE
  SerialGPS.begin(9600, SERIAL_8N1, BOARD_GPS_RXD, BOARD_GPS_TXD);
  bool result = false;
  uint32_t startTimeout;
  for (int i = 0; i < 3; ++i) {
    SerialGPS.write("$PCAS03,0,0,0,0,0,0,0,0,0,0,,,0,0*02\r\n");
    delay(5);
    // Get version information
    startTimeout = millis() + 3000;
    Serial.println("\n\n###########################\n[GPS]  Try to init L76K . Wait stop .");
    while (SerialGPS.available()) {
      Serial.print(".");
      Serial.println(SerialGPS.readString());
      if (millis() > startTimeout) {
        Serial.println("[GPS]  Wait L76K stop NMEA timeout!");
        return false;
      }
    };
    // Serial.println();
    SerialGPS.flush();
    delay(200);
    SerialGPS.write("$PCAS06,0*1B\r\n");
    startTimeout = millis() + 500;
    String ver = "";
    while (!SerialGPS.available()) {
      if (millis() > startTimeout) {
        Serial.println("Get L76K timeout!");
        return false;
      }
    }
    SerialGPS.setTimeout(10);
    ver = SerialGPS.readStringUntil('\n');
    Serial.print(ver);
    if (ver.startsWith("$GPTXT,01,01,02")) {
      Serial.println("[GPS]  L76K GNSS init succeeded, using L76K GNSS Module\n#######################");
      result = true;
      break;
    }
    delay(500);
  }
  // Initialize the L76K Chip, use GPS + Beidou
  SerialGPS.write("$PCAS04,3*1A\r\n");
  delay(250);
  Serial.println(SerialGPS.readStringUntil('\n'));
  // SerialGPS.write("$PCAS03,1,1,1,1,1,1,1,1,1,1,,,0,0*02\r\n");
  SerialGPS.write("$PCAS03,1,1,1,1,1,1,1,1,1,1,,,1,1,,,,1*33\r\n");
  delay(250);
  Serial.println(SerialGPS.readStringUntil('\n'));
  // Switch to Vehicle Mode, since SoftRF enables Aviation < 2g
  // SerialGPS.write("$PCAS11,3*1E\r\n");
  // Serial.println("[GPS]  Initializing the L76K Chip");
  return result;
}

bool gps_init() {
  bool result = false;
  // L76K GPS USE 9600 BAUDRATE
  result = setupGPS();
//   if (!result) {
//     // Set u-blox m10q gps baudrate 38400
//     SerialGPS.begin(38400, SERIAL_8N1, BOARD_GPS_RXD, BOARD_GPS_TXD);
//     result = GPS_Recovery();
//     if (!result) {
//       SerialGPS.updateBaudRate(9600);
//       result = GPS_Recovery();
//       if (!result) {
//         Serial.println("[GPS]  GPS Connect failed!");
//         result = false;
//       }
//       SerialGPS.updateBaudRate(38400);
//     }
//   }
  return true; //result;
}

static bool GPS_Recovery() {
  uint8_t cfg_clear1[] = {
    0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x02, 0x1C, 0xA2
  };
  uint8_t cfg_clear2[] = {
    0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x01, 0x1B, 0xA1
  };
  uint8_t cfg_clear3[] = {
    0xB5, 0x62, 0x06, 0x09, 0x0D, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
    0x00, 0x00, 0x03, 0x1D, 0xB3
  };
  SerialGPS.write(cfg_clear1, sizeof(cfg_clear1));
  if (getAck(buffer, 256, 0x05, 0x01)) {
    Serial.println("[GPS]  Got ACK success!");
  }
  SerialGPS.write(cfg_clear2, sizeof(cfg_clear2));
  if (getAck(buffer, 256, 0x05, 0x01)) {
    Serial.println("[GPS]  Got ACK success!");
  }
  SerialGPS.write(cfg_clear3, sizeof(cfg_clear3));
  if (getAck(buffer, 256, 0x05, 0x01)) {
    Serial.println("[GPS]  Got ACK success!");
  }
  // UBX-CFG-RATE, Size 8, 'Navigation/measurement rate settings'
  uint8_t cfg_rate[] = { 0xB5, 0x62, 0x06, 0x08, 0x00, 0x00, 0x0E, 0x30 };
  SerialGPS.write(cfg_rate, sizeof(cfg_rate));
  if (getAck(buffer, 256, 0x06, 0x08)) {
    Serial.println("[GPS]  Got ACK success!");
  } else {
    return false;
  }
  return true;
}

static int getAck(uint8_t *buffer, uint16_t size, uint8_t requestedClass, uint8_t requestedID) {
  uint16_t ubxFrameCounter = 0;
  bool ubxFrame = 0;
  uint32_t startTime = millis();
  uint16_t needRead;
  while (millis() - startTime < 800) {
    while (SerialGPS.available()) {
      int c = SerialGPS.read();
      switch (ubxFrameCounter) {
        case 0:
          if (c == 0xB5) {
            ubxFrameCounter++;
          }
          break;
        case 1:
          if (c == 0x62) {
            ubxFrameCounter++;
          } else {
            ubxFrameCounter = 0;
          }
          break;
        case 2:
          if (c == requestedClass) {
            ubxFrameCounter++;
          } else {
            ubxFrameCounter = 0;
          }
          break;
        case 3:
          if (c == requestedID) {
            ubxFrameCounter++;
          } else {
            ubxFrameCounter = 0;
          }
          break;
        case 4:
          needRead = c;
          ubxFrameCounter++;
          break;
        case 5:
          needRead |= (c << 8);
          ubxFrameCounter++;
          break;
        case 6:
          if (needRead >= size) {
            ubxFrameCounter = 0;
            break;
          }
          if (SerialGPS.readBytes(buffer, needRead) != needRead) {
            ubxFrameCounter = 0;
          } else {
            return needRead;
          }
          break;
        default:
          break;
      }
    }
  }
  return 0;
}

void showGPSinfo() {
  char text[32];
  epd_poweron();
  uint8_t yStep = 16;
  EpdRect myRect = {
    .x = 10,
    .y = 781,
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
  sprintf(text, "SIV: %d\n", SIV);
  printf(text);
  epd_write_string(font8b, text, &cx, &cy, fb, &font_props);
  cx = px;
  py += yStep;
  cy = py;
  if (validDate) {
    Serial.println(dateUTC);
    epd_write_string(font8b, dateUTC, &cx, &cy, fb, &font_props);
    cx = px;
    py += yStep;
    cy = py;
  }
  if (validTime) {
    Serial.println(timeUTC);
    epd_write_string(font8b, timeUTC, &cx, &cy, fb, &font_props);
    cx = px;
    py += yStep;
    cy = py;
  }
  if (validCoords) {
    sprintf(text, "%.6f,%.6f", latitude, longitude);
    Serial.println(text);
    epd_write_string(font8b, text, &cx, &cy, fb, &font_props);
    cx = px;
    py += yStep;
    cy = py;
    float distance = haversine(homeLat, homeLng, latitude, longitude);
    sprintf(text, "Distance %.2f m", distance);
    Serial.println(text);
    epd_write_string(font8b, text, &cx, &cy, fb, &font_props);
  }
  epd_poweroff();
  epd_poweron();
  checkError(epd_hl_update_screen(&hl, MODE_GL16, temperature));
  epd_poweroff();
}
