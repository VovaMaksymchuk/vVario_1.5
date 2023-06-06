#include <Adafruit_BMP280.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>

#include "U8glib.h"
#include <EEPROM.h>
#include <math.h>
// #include <VoltageReference.h>
#include <Tone.h>

// VoltageReference vRef;
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);

Adafruit_BMP280 sensor_bmp;

Tone tone_out1;
Tone tone_out2;

// #define buzzer 10
int speaker_pin1 = 9;   //arduino speaker output -
int speaker_pin2 = 10;  //arduino speaker output +

//buttons
const int button_1 = 2;
const int button_2 = 3;


// Timer
long timer;
int ms;
int seconds;
int minutes;
int hours;

// Vario
unsigned char samples = 15;
unsigned char maxsamples = 30;

float delta = 0.3;

float alt[51];
float tim[51];
float beep;
float Beep_period;

float Alt = 0;
float vario = 0;
String vario_display = " 0.0";

boolean atl_ready = false;

float Altitude = 0;
float alt_arr[10];
int alt_count = 0;

float Altitude_0 = 0;
float alt_0[10];
int alt_0_count = 0;

float Max_Vario_UP = 0.0;
float Max_Vario_DOWN = 0.0;
float Max_Altitude = 0.0;


// EEPROM ---------------------------------------------------
float vario_up_arr[] = { 0.35, 0.5, 0.8, 1.0, 1.3, 1.5, 100 };
float vario_down_arr[] = { -0.7, -1, -1.2, -1.35, -1.5, -2, -100 };

struct Settings {
  int up;
  int down;
};

Settings settings;

//page ctrl
int pageCtrl = 0;


// Setup ------------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);

  pinMode(button_1, INPUT_PULLUP);
  pinMode(button_2, INPUT_PULLUP);
  // pinMode(button_3, INPUT_PULLUP);

  sensor_bmp.begin();

  tone_out1.begin(speaker_pin1);
  tone_out2.begin(speaker_pin2);

  play_welcome_beep();

  if (EEPROM.read(0) == 255) {
    settings = (Settings){ 1, 2 };
    EEPROM.put(0, settings);
  } else {
    EEPROM.get(0, settings);
  }
}

// Display -----------------------------------------------------------------------------------------------
void Loading() {
  // u8g.setColorIndex(1);
  // u8g.drawBitmapP(0, 0, 40, 40, loading);
  u8g.setFont(u8g_font_8x13r);
  u8g.setPrintPos(20, 15);
  u8g.print("Lucky Flight");

  u8g.setPrintPos(30, 60);
  u8g.print("ver. 1.1");
}

void Home() {
  u8g.setFont(u8g_font_8x13r);
  u8g.setPrintPos(2, 10);

  //frames
  u8g.drawLine(5, 45, 123, 45);

  //alt
  u8g.setPrintPos(5, 64);
  if (abs(Altitude - Altitude_0) > delta) {
    u8g.print(round(Altitude - Altitude_0));
  } else {
    u8g.print(0);
  }

  u8g.print("m");

  //timer
  u8g.setPrintPos(70, 64);
  if (hours > 0) {
    u8g.print(hours);
    u8g.print(":");
  } else {
    u8g.print("0:");
  }

  if (minutes > 0) {
    if (minutes < 10) {
      u8g.print("0");
    }
    u8g.print(minutes);
    u8g.print(":");
  } else {
    u8g.print("00:");
  }
  if (seconds < 10) {
    u8g.print("0");
  }
  u8g.print(seconds);

  //vario
  //ms
  u8g.setPrintPos(82, 35);
  u8g.print("m/s");

  //value
  u8g.setScale2x2();
  u8g.setPrintPos(2, 18);
  if (atl_ready && abs(vario) > delta) {
    if (vario > 0) {
      u8g.print("+");
      u8g.print(vario, 1);
    } else {
      u8g.print(vario, 1);
    }
  } else {
    u8g.print(" 0.0");
  }

  u8g.undoScale();

  // setts


  // triangle
  // u8g.setFont(u8g_font_10x20_75r);
  u8g.setRot90();
  u8g.setPrintPos(2, 105);
  // u8g.print(">");
  if (vario > 0.3) { u8g.print("<"); }
  if (vario < -0.3) { u8g.print(">"); }

  u8g.setPrintPos(2, 90);
  //  u8g.print(">");
  if (vario > 1) { u8g.print("<"); }
  if (vario < -1) { u8g.print(">"); }

  u8g.setPrintPos(2, 75);
  //  u8g.print(">");
  if (vario > 2) { u8g.print("<"); }
  if (vario < -2) { u8g.print(">"); }
  u8g.undoRotation();

  // vcc
  // int vcc = 3.3;
  float Voltage;
  Voltage = analogRead(A0) * (3.3 / 1023.0);
  
  u8g.drawFrame(101, 3, 22, 10);
  u8g.drawFrame(123, 5, 2, 5);
  
  if (Voltage * 2 > 3.6) {
    u8g.drawBox(104, 5, 4, 6);
  }
  if (Voltage * 2 > 3.75) {
    u8g.drawBox(110, 5, 4, 6);
  }
  if (Voltage * 2 > 4) {
    u8g.drawBox(116, 5, 4, 6);
  }
}

void Stats() {
  u8g.setFont(u8g_font_8x13r);


  u8g.drawLine(5, 30, 123, 30);
  u8g.drawLine(42, 5, 42, 30);
  u8g.drawLine(84, 5, 84, 59);

  // vario
  u8g.setPrintPos(3, 15);
  u8g.print("+");
  if (abs(Max_Vario_UP) > delta) {
    u8g.print(Max_Vario_UP, 1);
  } else {
    u8g.print("0.0");
  }

  u8g.setPrintPos(46, 15);
  if (abs(vario) > delta) {
    if (vario > 0) {
      u8g.print("+");
      u8g.print(vario, 1);
    } else {
      u8g.print(vario, 1);
    }
  } else {
    u8g.print("0.0");
  }


  u8g.setPrintPos(89, 15);
  if (abs(Max_Vario_DOWN) > delta) {
    u8g.print(Max_Vario_DOWN, 1);
  } else {
    u8g.print("-0.0");
  }

  u8g.setFont(u8g_font_6x12);

  u8g.setPrintPos(12, 25);
  u8g.print("max");

  u8g.setPrintPos(55, 25);
  u8g.print("m/s");

  u8g.setPrintPos(97, 25);
  u8g.print("min");

  // alt
  u8g.setPrintPos(3, 45);
  u8g.print("now:");

  u8g.setPrintPos(3, 60);
  u8g.print("max:");

  u8g.setFont(u8g_font_8x13r);
  u8g.setPrintPos(30, 45);
  u8g.print(round(Altitude - Altitude_0));
  u8g.print("m");

  u8g.setPrintPos(30, 60);
  u8g.print(round(Max_Altitude));
  u8g.print("m");

  // time
  u8g.setFont(u8g_font_8x13r);
  u8g.setPrintPos(89, 48);
  if (hours > 0) {
    u8g.print(hours);
    u8g.print(":");
  } else {
    u8g.print("0:");
  }

  if (minutes > 0) {
    if (minutes < 10) {
      u8g.print("0");
    }
    u8g.print(minutes);
  } else {
    u8g.print("00");
  }
  u8g.setFont(u8g_font_6x12);
  u8g.setPrintPos(89, 58);
  u8g.print(seconds);
}


void Sensitivity() {
  u8g.setFont(u8g_font_6x12);
  u8g.setPrintPos(2, 10);
  u8g.print("Sensitivity");
  u8g.drawLine(2, 12, 124, 12);
  // u8g.drawLine(64, 14, 64, 62);

  u8g.setFont(u8g_font_9x15B);

  if (pageCtrl == 2) {
    u8g.setPrintPos(55, 35);
    u8g.print("UP");
    u8g.setPrintPos(40, 50);

    if (settings.up != 6) {
      u8g.print("+");
      u8g.print(vario_up_arr[settings.up]);
    } else {
      u8g.setPrintPos(55, 50);
      u8g.print("-");
    }
  } else {
    u8g.setPrintPos(45, 35);
    u8g.print("Down");
    u8g.setPrintPos(40, 50);

    if (settings.down != 6) {
      // u8g.print("-");
      u8g.print(vario_down_arr[settings.down]);
    } else {
      u8g.setPrintPos(55, 50);
      u8g.print("-");
    }
  }
}

void btn_a() {
  if (pageCtrl < 3) {
    pageCtrl++;
  } else {
    pageCtrl = 0;
  }
}

void btn_b() {
  if (pageCtrl == 2) {
    if (settings.up < 6) {
      settings.up++;
    } else {
      settings.up = 0;
    }
    EEPROM.put(0, settings);
  }

  if (pageCtrl == 3) {
    if (settings.down < 6) {
      settings.down++;
    } else {
      settings.down = 0;
    }
    EEPROM.put(0, settings);
  }
}

// Beeps ----------------------------------------------------------------------------------------
// void play_beep() {
//   for (int aa = 100; aa <= 800; aa = aa + 100) {
//     tone_out1.play(aa, 200);
//     tone_out2.play(aa + 3, 200);
//     delay(50);
//   }
// }

void play_welcome_beep() {
  for (int aa = 100; aa <= 800; aa = aa + 100) {
    tone_out1.play(aa, 200);
    tone_out2.play(aa + 3, 200);
    delay(50);
  }
}

void vario_beep() {
  if ((timer - beep) > Beep_period) {
    beep = timer;
    if (vario > vario_up_arr[settings.up] && vario < 15) {
      Beep_period = 350 - (vario * 5);
      tone_out1.play((1000 + (100 * vario)), 300 - (vario * 5));  // up
      tone_out2.play((1003 + (100 * vario)), 300 - (vario * 5));
    } else if (vario < vario_down_arr[settings.down]) {  // down
      Beep_period = 200;
      tone_out1.play((500 - (vario)), 340);
      tone_out2.play((503 - (vario)), 340);
    }
  }
}

// Loop ---------------------------------------------------

void loop() {
  u8g.firstPage();

  // // buttons ---------------------------------------------------
  // presentState = digitalRead(button_1);

  // if (previousState == HIGH && presentState == LOW && ON_Duration == false) {
  //   press_Time = millis();
  //   ON_Duration = true;
  // }
  // if (ON_Duration == true) {
  //   release_Time = millis();
  //   long ON_TIME = release_Time - press_Time;
  //   if (ON_TIME > LONG_PRESS) {
  //     ON_Duration = false;
  //     btn_long_back();
  //   }
  // }
  // if (previousState == LOW && presentState == HIGH && ON_Duration == true) {
  //   release_Time = millis();
  //   ON_Duration = false;
  //   long ON_TIME = release_Time - press_Time;
  //   if (ON_TIME < LONG_PRESS) {
  //     btn_back();
  //   }
  // }
  // previousState = presentState;
  if (digitalRead(button_1) == LOW) {
    btn_a();
    delay(200);
  }
  if (digitalRead(button_2) == LOW) {
    btn_b();
    delay(200);
  }
  // if (digitalRead(button_3) == LOW) {
  //   btn_b();
  //   delay(200);
  // }

  // timer ----------------------------------------------------------------------------------------
  timer = millis();
  long t = timer;
  ms = t % 1000;
  t /= 1000;
  seconds = t % 60;
  t /= 60;
  minutes = t % 60;
  t /= 60;
  hours = t % 24;

  // Vario -----------------------------------------------------------------------------

  float tempo = millis();
  float N1 = 0;
  float N2 = 0;
  float N3 = 0;
  float D1 = 0;
  float D2 = 0;

  Altitude = (sensor_bmp.readAltitude(1013));
  // // Serial.println(Altitude);

  if (alt_0_count <= 10) {
    Altitude_0 = Altitude;
    alt_0[alt_0_count] = Altitude;
    alt_0_count++;
  }

  if (alt_0_count == 10 && !atl_ready) {
    for (int i = 0; i <= 10; i++) {
      Altitude_0 += alt_0[i];
    }
    Altitude_0 = Altitude_0 / 10;
  }

  if (!atl_ready && abs(vario) < delta && tempo > 1000) {
    atl_ready = true;
  }


  for (int cc = 1; cc <= maxsamples; cc++) {  // average
    alt[(cc - 1)] = alt[cc];
    tim[(cc - 1)] = tim[cc];
  };

  alt[maxsamples] = Altitude;
  tim[maxsamples] = timer;

  float stime = tim[maxsamples - samples];
  for (int cc = (maxsamples - samples); cc < maxsamples; cc++) {
    N1 += (tim[cc] - stime) * alt[cc];
    N2 += (tim[cc] - stime);
    N3 += (alt[cc]);
    D1 += (tim[cc] - stime) * (tim[cc] - stime);
    D2 += (tim[cc] - stime);
  };

  vario = 1000 * ((samples * N1) - N2 * N3) / (samples * D1 - D2 * D2);

  // int vario_int = round(vario);
  // int vario_dec = round((vario - vario_int) * 10);
  // if (abs(vario) > delta) {
  //   if (vario < 0) {
  //     vario_display = "-";
  //   } else {
  //     vario_display = "+";
  //   }
  //   vario_display = vario_display + abs(vario_int) + "." + abs(vario_dec);
  // } else {
  //   vario_display = " 0.0";
  // }

  //--------------max vario-----------
  if (atl_ready) {
    if (atl_ready && vario > Max_Vario_UP) { Max_Vario_UP = vario; }
    if (atl_ready && vario < Max_Vario_DOWN) { Max_Vario_DOWN = vario; }

    if (atl_ready && Altitude - Altitude_0 > Max_Altitude) {
      Max_Altitude = Altitude - Altitude_0;
    }

    vario_beep();
  }

  // Beeper --------------------------------------------------------------


  //----------- screen -----------------------------------------------------------------------------
  do {
    if (atl_ready) {
      if (pageCtrl == 1) {
        Stats();
      } else if (pageCtrl > 1) {
        Sensitivity();
      } else {
        Home();
      }
    } else {
      Loading();
    }



  } while (u8g.nextPage());
}