#include <mpu6050_esp32.h>
#include<math.h>
#include<string.h>
#include "FS.h"
#include "SD.h"
#include <TFT_eSPI.h> 
#include <SPI.h>
#include <WiFi.h>
TFT_eSPI tft = TFT_eSPI();

const int SCREEN_HEIGHT = 160;
const int SCREEN_WIDTH = 128;

MPU6050 imu;
uint32_t primary_timer = 0;
float x, y, z;
float old_x, old_y, old_z;
float older_x, older_y, older_z;
float avg_x, avg_y, avg_z;
const float ZOOM = 9.81; // to convert units of accelerometer reading from g to m/s^2

const uint8_t BUTTON1 = 0;
const uint8_t BUTTON2 = 33;
const uint8_t BUTTON3 = 32;
const uint8_t BUTTON4 = 3;
uint8_t R = 12;
uint8_t G = 27;
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
float dance_time = 0;

// Dance state variables 
int punch_state = 0; // Move 1
int hand_roll_state = 0; // Move 2
int wave_state = 0; // Move 3
int bounce_state = 0; // Move 4
int sprinkler_state = 0; // Move 5
int arm_cross_state = 0; // Move 6
int disco_state = 0; // Move 7
int move_iter = 0;

// Game state variables
int game_state = 0;
int finish_state = 0;
int screen_state = 0;
int song_pick_state = 0;

// Game booleans
bool init_screen = true;
bool home_screen = true;
bool end_screen = false;
bool new_move = true;
bool username_selected = false;
bool song_selected = false;
bool begin_dance = false;
bool begin_rhythm = false;
bool begin_karaoke = false;
bool game_loaded = false;

// Dance/song selection variables
int selected_game = 0; // 1 is Just Dance, 2 is Rhythm Game
int selected_song = 0; // 1 is Stereo Hearts, 2 is Riptide 
const char *song_names[2] = {"Stereo Hearts", "Riptide"}; 

const uint16_t GREEN = 0x07e0;
const uint16_t BLACK = 0x0000;
const uint16_t MAROON = 0x7800;
const uint16_t PURPLE = 0x780F;
const uint16_t BLUE = 0x001F;
const uint16_t RED = 0xF800;
const uint16_t MAGENTA = 0xF81F;
const uint16_t ORANGE = 0xFDA0;
const uint16_t WHITE = 0xFFFF;
const uint16_t YELLOW = 0xFFE0;
const uint16_t CYAN = 0x07FF;
const uint16_t LIGHT_GRAY = 0xBDF7;
const uint16_t DARK_GRAY = 0x7BEF;

int step_num = 0;
int song_state = 0;
int song_index = 0;

// SD card
char message_buffer[9000] = "";

struct Riff {
  double notes[768]; //the notes (array of doubles containing frequencies in Hz. I used https://pages.mtu.edu/~suits/notefreqs.html
  int length; //number of notes (essentially length of array.
  float note_period; //the timing of each note in milliseconds (take bpm, scale appropriately for note (sixteenth note would be 4 since four per quarter note) then
};

struct Choreo {
  int steps; // number of steps in choreo
  int moves[20]; // sequence of dance moves
  int timing[20]; // number of beats per move 
  int counts[20]; // number of iterations of each move
};

// BOUNCE (8), HAND ROLL (8), PUNCH (8), SPRINKLER (8)
Choreo stereo_basic = {
  4, {4, 2, 1, 5}, {8, 8, 8, 8}, {8, 8, 8, 8}
};

// HAND ROLL (4), DISCO (2), PUNCH (4), WAVE (4), SPRINKLER (4), ARM CROSS (2)
Choreo stereo_advanced = {
  6, {2, 7, 1, 3, 5, 6}, {4, 4, 8, 8, 4, 4}, {4, 2, 4, 4, 4, 2}
};

// PUNCH (8), HAND ROLL (8), WAVE (8), BOUNCE (16), SPRINKLER (8), ARM CROSS (12), DISCO (16)
Choreo riptide_basic = {
  7, {1, 2, 3, 4, 5, 6, 7}, {16, 16, 16, 16, 16, 32, 27}, {8, 8, 8, 16, 8, 12, 16}
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

Riff riptide = {{466.16, 466.16, 523.25, 523.25, 554.37, 554.37, 622.25, 698.46, 698.46, 698.46, 932.33, 932.33, 830.61, 
830.61, 739.99, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 466.16, 466.16, 523.25, 523.25, 
554.37, 554.37, 622.25, 698.46, 698.46, 698.46, 932.33, 932.33, 830.61, 830.61, 739.99, 739.99, 698.46, 698.46, 622.25, 
622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 698.46, 698.46, 830.61, 830.61, 830.61, 830.61, 830.61, 698.46, 698.46, 
698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 622.25, 622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 
698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 0, 0, 0, 0, 698.46, 622.25, 698.46, 622.25, 622.25, 698.46, 
698.46, 622.25, 622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 622.25, 554.37, 554.37, 
554.37, 554.37, 554.37, 554.37, 554.37, 554.37, 0, 0, 0, 0, 0, 415.3, 415.3, 932.33, 932.33, 932.33, 932.33, 932.33, 
932.33, 932.33, 932.33, 830.61, 830.61, 830.61, 830.61, 830.61, 830.61, 830.61, 830.61, 698.46, 698.46, 698.46, 698.46, 
698.46, 698.46, 698.46, 698.46, 0, 0, 0, 0, 0, 0, 415.3, 415.3, 932.33, 932.33, 932.33, 932.33, 932.33, 932.33, 932.33, 
932.33, 830.61, 830.61, 830.61, 830.61, 830.61, 830.61, 830.61, 830.61, 932.33, 932.33, 932.33, 830.61, 830.61, 830.61, 
932.33, 932.33, 932.33, 830.61, 830.61, 830.61, 0, 0, 0, 0, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 698.46, 
698.46, 698.46, 830.61, 830.61, 698.46, 698.46, 622.25, 622.25, 622.25, 622.25, 554.37, 554.37, 554.37, 0, 0, 698.46, 
698.46, 698.46, 830.61, 830.61, 698.46, 698.46, 622.25, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 698.46, 
698.46, 698.46, 830.61, 830.61, 698.46, 698.46, 622.25, 622.25, 622.25, 622.25, 554.37, 554.37, 466.16, 554.37, 554.37, 
554.37, 554.37, 0, 0, 0, 0, 554.37, 554.37, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 698.46, 698.46, 698.46, 
830.61, 830.61, 698.46, 698.46, 622.25, 622.25, 622.25, 622.25, 554.37, 554.37, 554.37, 0, 0, 698.46, 698.46, 698.46, 
830.61, 830.61, 698.46, 622.25, 622.25, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 698.46, 698.46, 698.46, 
830.61, 830.61, 698.46, 622.25, 622.25, 554.37, 554.37, 554.37, 554.37, 554.37, 554.37, 554.37, 554.37, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 554.37, 554.37, 
554.37, 698.46, 698.46, 622.25, 622.25, 622.25, 554.37, 554.37, 554.37, 698.46, 698.46, 622.25, 622.25, 622.25, 622.25, 
622.25, 622.25, 622.25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 554.37, 554.37, 554.37, 698.46, 698.46, 622.25, 
622.25, 622.25, 554.37, 554.37, 554.37, 698.46, 698.46, 466.16, 466.16, 466.16, 466.16, 466.16, 466.16, 466.16, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 554.37, 554.37, 554.37, 698.46, 698.46, 622.25, 622.25, 622.25, 554.37, 554.37, 
554.37, 698.46, 698.46, 622.25, 622.25, 622.25, 622.25, 622.25, 622.25, 622.25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 554.37, 554.37, 554.37, 698.46, 698.46, 622.25, 622.25, 622.25, 554.37, 554.37, 554.37, 698.46, 698.46, 622.25, 
622.25, 622.25, 622.25, 622.25, 622.25, 554.37, 554.37, 554.37, 466.16, 466.16, 466.16, 554.37, 554.37, 554.37, 554.37, 
554.37, 554.37, 554.37, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 622.25, 622.25, 622.25, 
698.46, 698.46, 622.25, 622.25, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 698.46, 0, 0, 0, 0, 0, 0, 0, 0, 
698.46, 698.46, 622.25, 622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 622.25, 622.25, 622.25, 698.46, 698.46, 622.25, 
622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 622.25, 622.25, 622.25, 698.46, 698.46, 698.46, 698.46, 0, 0, 0, 0}, 556, 150.00};

Riff song_to_play = stereo;
Choreo dance_to_play = stereo_basic;

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
char user[10];
int just_dance_total = 0;
int rhythm_score = 0;
char individual_scores[500] = "";

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char old_response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
const int LOOP_PERIOD = 20;

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
    } else if (state==2) {
      if (button_pressed and millis() - state_2_start_time >= long_press_duration) {
        state = 3;
      } else if (!button_pressed) {
        state = 4;
        button_change_time = millis();
      }
    } else if (state==3) {
      if (!button_pressed) {
        state = 4;
        button_change_time = millis();
      }
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
    }
    return flag;
  }
};

Button button(BUTTON1);
Button button2(BUTTON2);
Button button3(BUTTON3);
Button button4(BUTTON4);
Button buttons[4] = {button, button2, button3, button4};

class Note {
  public:
    uint8_t action;    // which button (1, 2, 3, 4)
    uint8_t type;      // short press vs hold. Currently unused; hold notes aren't implemented yet. implement later if there's time
    uint32_t duration; // duration of hold note; also currently unused
    int start_ind;
    bool active;
    TFT_eSPI* local_tft;
    int NOTE_CLR; // idk if you wanna have different colors. actually just check if any of the stuff below here is necessary
    int BKGND_CLR;
    int RADIUS;
    int DT;
    float y;
    float v;
    float g;
    Note(TFT_eSPI* tft_to_use=&tft, uint8_t a=0, int start=0, uint8_t t=1, int d=0, int rad = 6, int dt=LOOP_PERIOD, 
        int note_color = TFT_BLUE, int backgnd_color = TFT_BLACK) { // value of dt might be kinda sketch
          
        action = a;
        type = t;
        start_ind = start;
        duration = d;
        active = true;
        local_tft = tft_to_use;
        RADIUS = rad;
        NOTE_CLR = note_color;
        BKGND_CLR = backgnd_color;
        DT = dt;
        y = -RADIUS;
        v = 60;
        g = 50; // prev 16 10 w/o loop period
    }
    void step() {
      if (active) {
        local_tft->drawCircle(25*(action+1), y, RADIUS, BKGND_CLR);
        v += 0.001*float(DT)*g;
        y += v*0.001*float(DT);
        local_tft->drawCircle(25*(action+1), y, RADIUS, NOTE_CLR);
      } 

    }
    void checkHit() {
      if (y > SCREEN_HEIGHT + RADIUS) {
        Serial.println("miss");
        active = false;
        digitalWrite(R, HIGH);
        digitalWrite(G, LOW);
      }
      int val = buttons[action].update();
      if (val == 1 && active) {
        uint32_t off_timing = abs(millis() - song_timer - start_ind*song_to_play.note_period);
        //Serial.println(off_timing);
        if (off_timing <= 80) {
          Serial.println("perfect!");
          digitalWrite(R, LOW);
          digitalWrite(G, HIGH);
          rhythm_score += 100;
        } else if (off_timing <= 160) {
          Serial.println("good");
          digitalWrite(R, HIGH);
          digitalWrite(G, HIGH);
          rhythm_score += 50;
        } else {
          Serial.println("miss");
          digitalWrite(R, HIGH);
          digitalWrite(G, LOW);
        }
        //Serial.println(rhythm_score);
        active = false;
        local_tft->drawCircle(25*(action+1), y, RADIUS, BKGND_CLR);       
      }
    }
};

struct Beatmap {
  int actions[100]; // which button to press (0, 1, 2, 3)
  int indices[100]; // index of the note in the Riff that this button should be pressed on
  int num_notes; // total number of notes player should hit
};

Beatmap stereo_map = {{ 0,  1,  3,  2,  3,  2,  1,  0,  1,  2,  0,  1,  2,  3,  1,  0,  1,  3,  2,  3,  2,  1,  0,  1,  2},
                      {13, 13, 20, 22, 23, 25, 27, 29, 36, 36, 47, 47, 47, 52, 54, 77, 84, 86, 87, 89, 91,100,111,111,111}, 25};

Beatmap map_to_play = stereo_map;
Note map_notes[200]; // change length if needed                 

class UsernameGetter {
    char alphabet[50] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char msg[400] = {0}; //contains previous query response
    char query_string[50] = {0};
    int char_index;
    int state;
    int query_index = 0;
    uint32_t scrolling_timer;
    const int scrolling_threshold = 150;
    const float angle_threshold = 0.3 * ZOOM;
  public:
    UsernameGetter() {
      state = 0;
      memset(msg, 0, sizeof(msg));
      strcat(msg, "Press & tilt!");
      char_index = 0;
      scrolling_timer = millis();
    }
    void update(float angle, int button, char* output) {
      switch(state) {
        case 0:
          strcpy(output, msg);
          if (button == 2){
            state = 1;
            char_index = 0;
            scrolling_timer = millis();
          }
          break;
        case 1:
          if (button == 1) {
            strncat(query_string, &alphabet[char_index], 1);
            char_index = 0;
            strcpy(output, query_string);
            strncat(output, &alphabet[char_index], 1);
          } else if (button == 2) {
            state = 2;
            strcpy(user, output);
            memset(output, 0, sizeof(output));
          } else if (millis() - scrolling_timer > scrolling_threshold) {
            if (abs(angle) > angle_threshold) {
              if (angle > 0) {
                char_index ++;
                char_index = char_index % strlen(alphabet);
              } else {
                char_index --;
                if (char_index == -1) {
                  char_index = strlen(alphabet) - 1;
                }
              }
              scrolling_timer = millis();
            }
            strcpy(output, query_string);
            strncat(output, &alphabet[char_index], 1);
          } else {
            strcpy(output, query_string);
            strncat(output, &alphabet[char_index], 1);
          }
          break;
        case 2:
          username_selected = true;
          state = 0;
          tft.fillScreen(BLACK);
          Serial.println("FINISHED COLLECTING USERNAME");
          break;
      }
    }
};

UsernameGetter ug;

void setup() {
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
  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(BUTTON4, INPUT_PULLUP);
  pinMode(AUDIO_TRANSDUCER, OUTPUT);
  pinMode(LCD_CONTROL, OUTPUT);
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);

  SPIClass spi = SPIClass(VSPI);
  spi.begin(14, 19, 13, 5); // SCK, MISO, MOSI, CS

  if(!SD.begin(5, spi)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
      Serial.println("MMC");
  } else if(cardType == CARD_SD){
      Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
  } else {
      Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
//    writeFile(SD, "/toy_internal.txt", "440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440,440");
  readFile(SD, "/havana.txt");
  Serial.println(message_buffer);
  listDir(SD, "/", 0);
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  double_extractor(message_buffer, song_to_play.notes, ',');

  //set up AUDIO_PWM which we will control for music:
  ledcSetup(AUDIO_PWM, 0, 12);//12 bits of PWM precision
  ledcWrite(AUDIO_PWM, 0); //0 is a 0% duty cycle for the NFET
  ledcAttachPin(AUDIO_TRANSDUCER, AUDIO_PWM);

  timer = millis();
  song_timer = millis();

  game_end = 0;
  just_dance_total = 0;
  rhythm_score = 0;
  song_index = 0;
  draw_user_selection_screen();
}

void loop() {  
  int bv = button.update();
  int b2 = button2.update();
  read_accel();

  if (!username_selected) {
    get_username(b2);
  }
  
  if (username_selected && init_screen) {
    Serial.println("clause 1");
    draw_home_screen();
    init_screen = false;
  }
  
  if (username_selected && home_screen) {
    Serial.println("clause 2");
    select_game(bv); 
  }

  if (end_screen) {
    Serial.println("clause 3");
    finish_game(bv);
  }

  if (begin_dance && selected_game == 1) {
    Serial.println("clause 4");
    play_just_dance();
  }

  if (begin_rhythm && selected_game == 2) {
    Serial.println("clause 5");
    play_rhythm_game();
  }

  if (begin_karaoke && selected_game == 3) {
    Serial.println("clause 5");
    play_karaoke_game();
  }
 
  if (game_end) { // POST state
    Serial.println("you made it to the end!");
    char thing[500];
    if (selected_game == 1) {
      sprintf(thing, "user=%s&justdance=%i&rhythm=0&karaoke=0", user, just_dance_total);
    } else if (selected_game == 2) {
      sprintf(thing, "user=%s&justdance=0&rhythm=%d&karaoke=0", user, rhythm_score);
      rhythm_score = 0;
    }
    post_score(thing);
    game_end = false;
    reset_dance_states();
    new_move = true;
  }
}

void read_accel() {
  imu.readAccelData(imu.accelCount);
  x = imu.accelCount[0] * imu.aRes * ZOOM;
  y = imu.accelCount[1] * imu.aRes * ZOOM;
  z = imu.accelCount[2] * imu.aRes * ZOOM;
}

//void post_score(int score) {
void post_score(char* thing) {
  Serial.println("the game ended!");
  //char thing[500];
  //sprintf(thing, "user=%s&justdance=%i&rhythm=0&karaoke=0", user, score);      
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

void get_username(int b2) {
  ug.update(-y, b2, response);
  if (strcmp(response, old_response) != 0) {//only draw if changed!
    tft.fillRect(15, 86, 98, 13, BLACK);
    tft.setCursor(17, 89, 1);
    tft.println(response);
  }
  memset(old_response, 0, sizeof(old_response));
  strcat(old_response, response);
  while (millis() - primary_timer < LOOP_PERIOD); //wait for primary timer to increment
  primary_timer = millis();
}

void add_stars(int result) {
  Serial.println("here!");
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

void play_just_dance() {
  if (step_num < dance_to_play.steps && millis() - song_timer > dance_time) {
      int result = similarity_score(dance_to_play.counts[step_num], move_iter);
      new_move = true;
      step_num += 1;
      dance_time += time_per_beat * dance_to_play.timing[step_num];
      add_stars(result);
      just_dance_total += result; // add to running total score
      move_iter = 0;
      tft.fillRect(0, 0, 128, 20, BLACK);
    } else if (step_num == dance_to_play.steps) { // reset all values, enter game end state
      game_end = true;
      song_state = 0;
      new_move = true;
      begin_dance = false;
      begin_rhythm = false;
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
  
    if (dance_to_play.moves[step_num] == 1) {
      if (new_move) {
        sprintf(individual_scores, "%s%s", individual_scores, "Punch");
        Serial.println(individual_scores);
        new_move = false;
      }
      punch();
    } else if (dance_to_play.moves[step_num] == 2) {
      if (new_move) {
        sprintf(individual_scores, "%s%s", individual_scores, "Hand Roll");
        Serial.println(individual_scores);
        new_move = false;
      }
      hand_roll();
    } else if (dance_to_play.moves[step_num] == 3) {
      if (new_move) {
        sprintf(individual_scores, "%s%s", individual_scores, "Wave");
        Serial.println(individual_scores);
        new_move = false;
      }
      wave();
    } else if (dance_to_play.moves[step_num] == 4) {
      if (new_move) {
        sprintf(individual_scores, "%s%s", individual_scores, "Bounce");
        Serial.println(individual_scores);
        new_move = false;
      }
      bounce();
    } else if (dance_to_play.moves[step_num] == 5) {
      if (new_move) {
        sprintf(individual_scores, "%s%s", individual_scores, "Sprinkler");
        Serial.println(individual_scores);
        new_move = false;
      }
      sprinkler();
    } else if (dance_to_play.moves[step_num] == 6) {
      if (new_move) {
        sprintf(individual_scores, "%s%s", individual_scores, "Arm Cross");
        Serial.println(individual_scores);
        new_move = false;
      }
      arm_cross();
    } else if (dance_to_play.moves[step_num] == 7) {
      if (new_move) {
        sprintf(individual_scores, "%s%s", individual_scores, "Disco");
        Serial.println(individual_scores);
        new_move = false;
      }
      disco();
    }
}

void play_karaoke_game() {
  tft.fillScreen(BLACK);
  tft.drawString("Insert karaoke game", 10, 60, 1);
  delay(3000);
  home_screen = true;
  begin_rhythm = false;
  begin_dance = false;
  begin_karaoke = false;
  end_screen = false;
  tft.fillScreen(BLACK);
  draw_home_screen();
}

void play_rhythm_game() {
  //tft.drawString("Insert rhythm game", 10, 60, 1);
  //delay(3000);
  
  for (int i=0; i < map_to_play.num_notes; i++) {
    if (millis() - song_timer >= map_notes[i].start_ind*song_to_play.note_period - 1500 && 
        millis() - song_timer <= map_notes[i].start_ind*song_to_play.note_period + 200) { 
      map_notes[i].step(); 
      map_notes[i].checkHit();
    }
  }
  char score_str[20];
  sprintf(score_str, "Score: %d", rhythm_score);
  tft.setCursor(0,0,2);
  tft.println(score_str);

  if (millis() >= song_timer) {
    if (song_index == 0) {
        Serial.println("Start music!");
        ledcWriteTone(AUDIO_PWM, song_to_play.notes[song_index]);
        song_index++;
  
        song_timer = millis(); 
    } 
    if (millis() - song_timer >= song_index * song_to_play.note_period) { // time to switch to the next note
      if (song_index == song_to_play.length && begin_rhythm) { // end of song
        ledcWriteTone(AUDIO_PWM, 0);
        song_state = 0; 
        song_index = 0;

        game_end = true;
        begin_rhythm = false;
        begin_dance = false;
        rhythm_end();        
        
      } else if (song_to_play.notes[song_index] != song_to_play.notes[song_index-1]) { // note change
      ledcWriteTone(AUDIO_PWM, song_to_play.notes[song_index]);
      song_index ++;
      } else song_index++; // otherwise increment index 
    }
  }
  if (begin_rhythm) {
    tft.drawRect(0, SCREEN_HEIGHT-3*6, SCREEN_WIDTH, 3*6, TFT_WHITE); // visual indicator bar; hit notes when they are inside
  }
  
  while (millis() - primary_timer < LOOP_PERIOD);
  primary_timer = millis();
}
