#include <mpu6050_esp32.h>
#include<math.h>
#include<string.h>
#include <TFT_eSPI.h> 
#include <SPI.h>
#include <WiFi.h>
TFT_eSPI tft = TFT_eSPI();

MPU6050 imu;
uint32_t primary_timer = 0;
float x, y, z;
float old_x, old_y, old_z;
float older_x, older_y, older_z;
float avg_x, avg_y, avg_z;
const float ZOOM = 9.81; // to convert units of accelerometer reading from g to m/s^2

const uint8_t BUTTON1 = 5;
const uint8_t BUTTON2 = 19;
int db_state1 = 1;
int db_count1 = 0;
int db_state2 = 1;
int db_count2 = 0;
const uint8_t DB_COUNT_THRESHOLD = 100;
int tempo = 90;
const int readings_per_beat = 20;
const float delay_length = (60 * 1000) / (readings_per_beat * tempo);
const float time_per_beat = (60 * 1000) / tempo;
int training_trials = 1;
const int beats = 4;
uint32_t timer;
bool begin_game = false;
float dance_time = 0;
bool init_screen = true;
bool home_screen = true;
bool end_screen = false;
bool new_move = true;

int punch_state = 0; // Move 1
int hand_roll_state = 0; // Move 2
int wave_state = 0; // Move 3
int bounce_state = 0; // Move 4
int move_iter = 0;

int game_state = 0;
int finish_state = 0;
int selected_game = 0; // 1 is Just Dance, 2 is Rhythm Game

const uint16_t GREEN = 0x07e0;
const uint16_t BLACK = 0x0000;
const uint16_t MAROON = 0x7800;
const uint16_t PURPLE = 0x780F;
const uint16_t BLUE = 0x001F;
const uint16_t RED = 0xF800;
const uint16_t MAGENTA = 0xF81F;
const uint16_t ORANGE = 0xFDA0;
const uint16_t WHITE = 0xFFFF;

float choreo[4] = {1, 2, 3, 4};
float choreo_timing[4] = {36, 36, 36, 31};
int step_num = 0;

int song_state = 0;
int song_index = 0;

struct Riff {
  double notes[768]; //the notes (array of doubles containing frequencies in Hz. I used https://pages.mtu.edu/~suits/notefreqs.html
  int length; //number of notes (essentially length of array.
  float note_period; //the timing of each note in milliseconds (take bpm, scale appropriately for note (sixteenth note would be 4 since four per quarter note) then
};

uint32_t song_timer;

Riff stereo = {{0, 0, 0, 0, 880.0, 880.0, 880.0, 880.0, 830.61, 830.61, 739.99, 659.25, 659.25, 587.33, 587.33, 554.37, 
    554.37, 554.37, 554.37, 440, 659.25, 659.25, 739.99, 659.25, 659.25, 587.33, 587.33, 554.37, 554.37, 493.88, 493.88, 
    554.37, 554.37, 554.37, 554.37, 440, 659.25, 659.25, 739.99, 659.25, 659.25, 587.33, 587.33, 554.37, 554.37, 493.88, 
    493.88, 554.37, 554.37, 554.37, 554.37, 554.37, 659.25, 659.25, 493.88, 493.88, 493.88, 493.88, 493.88, 493.88, 0, 0, 0, 0, 
    0, 0, 0, 0, 880.0, 880.0, 880.0, 880.0, 830.61, 830.61, 739.99, 659.25, 659.25, 587.33, 587.33, 554.37, 554.37, 554.37, 554.37, 
    440, 659.25, 659.25, 739.99, 659.25, 659.25, 587.33, 587.33, 554.37, 554.37, 493.88, 493.88, 554.37, 554.37, 554.37, 554.37, 440, 
    659.25, 659.25, 739.99, 659.25, 659.25, 587.33, 587.33, 554.37, 554.37, 493.88, 493.88, 554.37, 554.37, 554.37, 554.37, 440, 
    659.25, 739.99, 739.99, 659.25, 659.25, 659.25, 554.37, 554.37, 554.37, 493.88, 440, 440}, 128, 166.67};

Riff riptide = {{466.16, 466.16, 523.25, 523.25, 554.37, 554.37, 622.25, 698.46, 698.46, 698.46, 932.33, 932.33, 830.61, 830.61, 739.99, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 466.16, 466.16, 523.25, 523.25, 554.37, 554.37, 622.25, 698.46, 698.46, 698.46, 932.33, 932.33, 830.61, 830.61, 739.99, 739.99, 698.46, 698.46, 622.25, 622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 698.46, 698.46, 830.61, 830.61, 830.61, 830.61, 830.61, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 622.25, 622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 0, 0, 0, 0, 698.46, 622.25, 698.46, 622.25, 622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 622.25, 554.37, 554.37, 554.37, 554.37, 554.37, 554.37, 554.37, 554.37, 0, 0, 0, 0, 0, 415.3, 415.3, 932.33, 932.33, 932.33, 932.33, 932.33, 932.33, 932.33, 932.33, 830.61, 830.61, 830.61, 830.61, 830.61, 830.61, 830.61, 830.61, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 0, 0, 0, 0, 415.3, 415.3, 932.33, 932.33, 932.33, 932.33, 932.33, 932.33, 932.33, 932.33, 830.61, 830.61, 830.61, 830.61, 830.61, 830.61, 830.61, 830.61, 932.33, 932.33, 932.33, 830.61, 830.61, 830.61, 932.33, 932.33, 932.33, 830.61, 830.61, 830.61, 0, 0, 0, 0, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 698.46, 698.46, 698.46, 830.61, 830.61, 698.46, 698.46, 622.25, 622.25, 622.25, 622.25, 554.37, 554.37, 554.37, 0, 0, 698.46, 698.46, 698.46, 830.61, 830.61, 698.46, 698.46, 622.25, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 698.46, 698.46, 698.46, 830.61, 830.61, 698.46, 698.46, 622.25, 622.25, 622.25, 622.25, 554.37, 554.37, 466.16, 554.37, 554.37, 554.37, 554.37, 0, 0, 0, 0, 554.37, 554.37, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 698.46, 698.46, 698.46, 830.61, 830.61, 698.46, 698.46, 622.25, 622.25, 622.25, 622.25, 554.37, 554.37, 554.37, 0, 0, 698.46, 698.46, 698.46, 830.61, 830.61, 698.46, 622.25, 622.25, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 698.46, 698.46, 698.46, 830.61, 830.61, 698.46, 622.25, 622.25, 554.37, 554.37, 554.37, 554.37, 554.37, 554.37, 554.37, 554.37, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 554.37, 554.37, 554.37, 698.46, 698.46, 622.25, 622.25, 622.25, 554.37, 554.37, 554.37, 698.46, 698.46, 622.25, 622.25, 622.25, 622.25, 622.25, 622.25, 622.25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 554.37, 554.37, 554.37, 698.46, 698.46, 622.25, 622.25, 622.25, 554.37, 554.37, 554.37, 698.46, 698.46, 466.16, 466.16, 466.16, 466.16, 466.16, 466.16, 466.16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 554.37, 554.37, 554.37, 698.46, 698.46, 622.25, 622.25, 622.25, 554.37, 554.37, 554.37, 698.46, 698.46, 622.25, 622.25, 622.25, 622.25,
622.25, 622.25, 622.25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 554.37, 554.37, 554.37, 698.46, 698.46, 622.25, 622.25, 622.25, 554.37, 554.37, 554.37, 698.46, 698.46, 622.25, 622.25, 622.25, 622.25, 622.25, 622.25, 554.37, 554.37, 554.37, 466.16, 466.16, 466.16, 554.37, 554.37, 554.37, 554.37, 554.37, 554.37, 554.37, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 622.25, 622.25, 622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 0, 0, 0, 0, 0, 0, 698.46, 698.46, 622.25, 622.25, 698.46,
698.46, 622.25, 622.25, 698.46, 622.25, 622.25, 622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 622.25, 622.25, 622.25, 698.46, 698.46, 698.46, 698.46, 0, 0, 0, 0}, 556, 150.00};

Riff song_to_play = riptide;

const uint32_t READING_PERIOD = 150; //milliseconds
double MULT = 1.059463094359; //12th root of 2 (precalculated) for note generation
double A1 = 55; //A1 55 Hz  for note generation
const uint8_t NOTE_COUNT = 97; //number of notes set at six octaves from

//pins for LCD and AUDIO CONTROL
uint8_t LCD_CONTROL = 25;
uint8_t AUDIO_TRANSDUCER = 26;

//PWM Channels. The LCD will still be controlled by channel 0, we'll use channel 1 for audio generation
uint8_t LCD_PWM = 0;
uint8_t AUDIO_PWM = 1;

//global variables to help your code remember what the last note was to prevent double-playing a note which can cause audible clicking
float new_note = 0;
float old_note = 0;
char freq_buffer[100];

//Code for sending POST request at the end

char network[] = "MIT";  //SSID for 6.08 Lab
char password[] = ""; //Password for 6.08 Lab

char host[] = "608dev-2.net";
char request[2000];
int game_end;
char* user = "testuser";
int just_dance_total = 0;
char individual_scores[500] = "";

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char old_response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request

class Button {
  public:
  uint32_t state_2_start_time;
  uint32_t button_change_time;    
  uint32_t debounce_duration;
  uint32_t long_press_duration;
  uint8_t pin;
  uint8_t flag;
  bool button_pressed;
  uint8_t state; // This is public for the sake of convenience
  Button(int p) {
  flag = 0;  
    state = 0;
    pin = p;
    state_2_start_time = millis(); //init
    button_change_time = millis(); //init
    debounce_duration = 10;
    long_press_duration = 1000;
    button_pressed = 0;
  }
  void read() {
    uint8_t button_state = digitalRead(pin);  
    button_pressed = !button_state;
  }
  int update() {
    read();
    flag = 0;
    if (state==0) {
      if (button_pressed) {
        state = 1;
        button_change_time = millis();
      }
    } else if (state==1) {
      if (!button_pressed) {
        state = 0;
        button_change_time = millis();
      }
      else if (button_pressed and millis() - button_change_time >= debounce_duration) {
        state = 2;
        state_2_start_time = millis();
      }
      // CODE HERE
    } else if (state==2) {
      if (button_pressed and millis() - state_2_start_time >= long_press_duration) {
        state = 3;
      } else if (!button_pressed) {
        state = 4;
        button_change_time = millis();
      }
      // CODE HERE
    } else if (state==3) {
      if (!button_pressed) {
        state = 4;
        button_change_time = millis();
      }
      // CODE HERE
    } else if (state==4) {
      if (!button_pressed and millis()-button_change_time >= debounce_duration) {
        state = 0;
        if (millis() - state_2_start_time < long_press_duration) {
          flag = 1;
          Serial.println("short press!");
        }
        else if (millis() - state_2_start_time >= long_press_duration) {
          flag = 2;
          Serial.println("long press!");
        }
      }
      else if (button_pressed and millis() - state_2_start_time < long_press_duration) {
        button_change_time = millis();
        state = 2;        
      }
      else if (button_pressed and millis() - state_2_start_time >= long_press_duration) {
        button_change_time = millis();
        state = 3;
      }
      // CODE HERE
    }
    return flag;
  }
};

Button button(BUTTON1);
Button button2(BUTTON2);

void setup() {
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  Serial.begin(115200);
  WiFi.begin(network, password); //attempt to connect to wifi
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                  WiFi.localIP()[1], WiFi.localIP()[0],
                  WiFi.macAddress().c_str() , WiFi.SSID().c_str());    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  delay(50);
  Wire.begin();
  delay(50); 
  if (imu.setupIMU(1)) {
//    Serial.println("IMU Connected!");
  } else {
//    Serial.println("IMU Not Connected :/");
//    Serial.println("Restarting");
    ESP.restart();
  }
  tft.init();
  tft.setRotation(2);
  primary_timer = millis();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(AUDIO_TRANSDUCER, OUTPUT);
  pinMode(LCD_CONTROL, OUTPUT);

  //set up AUDIO_PWM which we will control for music:
  ledcSetup(AUDIO_PWM, 0, 12);//12 bits of PWM precision
  ledcWrite(AUDIO_PWM, 0); //0 is a 0% duty cycle for the NFET
  ledcAttachPin(AUDIO_TRANSDUCER, AUDIO_PWM);

  timer = millis();
  song_timer = millis();

  game_end = 0;
  just_dance_total = 0;
  song_index = 0;
}

void loop() {  
  read_accel();

  if (init_screen) {
    draw_home_screen();
    init_screen = false;
  }
  
  int bv = button.update();
  int b2 = button2.update();
  
  if (home_screen) {
    select_game(bv); 
  }

  if (end_screen) {
    finish_game(bv);
  }
  
  if (game_end) { // POST state
    Serial.println("you made it to the end!");
    post_score(just_dance_total);
    game_end = false;
    reset_dance_states();
  }

  if (begin_game) {
    if (step_num < 4 && millis() - song_timer > dance_time) {
      int result = similarity_score(choreo_timing[step_num], move_iter);
      new_move = true;
      step_num += 1;
      dance_time += time_per_beat * choreo_timing[step_num];
      add_stars(result);
      just_dance_total += result; // add to running total score
      move_iter = 0;
      tft.fillRect(0, 0, 128, 20, BLACK);
    } else if (step_num == 4) { // reset all values, enter game end state
      game_end = true;
      song_state = 0;
      new_move = true;
      begin_game = false;
      step_num = 0;
      ledcWriteTone(AUDIO_PWM, 0);
      just_dance_end();
    }

    // music playing section
    if (song_index == 0) {
      Serial.println("Start music!");
      ledcWriteTone(AUDIO_PWM, song_to_play.notes[song_index]);
      song_index++;
      } 
      if (millis() - song_timer >= song_index * song_to_play.note_period) { // time to switch to the next note
        if (song_index == song_to_play.length) { // end of song
          ledcWriteTone(AUDIO_PWM, 0);
          song_state = 0; 
          song_index = 0;
        } else if (song_to_play.notes[song_index] != song_to_play.notes[song_index-1]) { // note change
          ledcWriteTone(AUDIO_PWM, song_to_play.notes[song_index]);
          song_index ++;
        } else song_index++; // otherwise increment index 
      }
  
    if (choreo[step_num] == 1) {
      if (new_move) {
        sprintf(individual_scores, "%s%s", individual_scores, "Punch");
        new_move = false;
      }
      punch();
    } else if (choreo[step_num] == 2) {
      if (new_move) {
        sprintf(individual_scores, "%s%s", individual_scores, "Hand Roll");
        new_move = false;
      }
      hand_roll();
    } else if (choreo[step_num] == 3) {
      if (new_move) {
        sprintf(individual_scores, "%s%s", individual_scores, "Wave");
        new_move = false;
      }
      wave();
    } else if (choreo[step_num] == 4) {
      if (new_move) {
        sprintf(individual_scores, "%s%s", individual_scores, "Bounce");
        new_move = false;
      }
      bounce();
    }
  }
}

void read_accel() {
  imu.readAccelData(imu.accelCount);
  x = imu.accelCount[0] * imu.aRes * ZOOM;
  y = imu.accelCount[1] * imu.aRes * ZOOM;
  z = imu.accelCount[2] * imu.aRes * ZOOM;
}

void post_score(int score) {
  Serial.println("the game ended!");
  char thing[500];
  sprintf(thing, "user=%s&justdance=%i&rhythm=0&karaoke=0", user, score);      
  sprintf(request, "POST http://608dev-2.net/sandbox/sc/team64/scoreboard.py HTTP/1.1\r\n");
  sprintf(request + strlen(request), "Host: %s\r\n", host);
  strcat(request, "Content-Type: application/x-www-form-urlencoded\r\n");
  sprintf(request + strlen(request), "Content-Length: %d\r\n\r\n", strlen(thing));
  strcat(request, thing);
  Serial.println(request);
  do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
}

int similarity_score(int correct, int actual) {
  if (actual > 0.9 * correct) {
    return 5;
  } else if (actual > 0.7 * correct) {
    return 4;
  } else if (actual > 0.5 * correct) {
    return 3;
  } else if (actual > 0.3 * correct) {
    return 2;
  } else if (actual > 0.1 * correct) {
    return 1;
  } else {
    return 0;
  }
}

void reset_dance_states() {
  punch_state = 0;
  hand_roll_state = 0;
  wave_state = 0;
  bounce_state = 0;
  move_iter = 0;
}

void add_stars(int result) {
  if (result == 0) {
    sprintf(individual_scores, "%s: %s\n", individual_scores, "/");
  } else if (result == 1) {
    sprintf(individual_scores, "%s: %s\n", individual_scores, "*");
  } else if (result == 2) {
    sprintf(individual_scores, "%s: %s\n", individual_scores, "**");
  } else if (result == 3) {
    sprintf(individual_scores, "%s: %s\n", individual_scores, "***");
  } else if (result == 4) {
    sprintf(individual_scores, "%s: %s\n", individual_scores, "****");
  } else if (result == 5) {
    sprintf(individual_scores, "%s: %s\n", individual_scores, "*****");
  }
}
