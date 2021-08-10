#include <mpu6050_esp32.h>
#include<math.h>
#include<string.h>
#include "FS.h"
#include "SD.h"
#include <TFT_eSPI.h> 
#include <SPI.h>
#include <WiFi.h>
TFT_eSPI tft = TFT_eSPI();

TaskHandle_t Task1;
TaskHandle_t Task2;

const int DELAY = 1000;
const int SAMPLE_FREQ = 10000;                          // Hz, telephone sample rate
const int SAMPLE_DURATION = 1;                        // duration of fixed sampling (seconds)
const int NUM_SAMPLES = SAMPLE_FREQ * SAMPLE_DURATION;  // number of of samples
const int ENC_LEN = (NUM_SAMPLES + 2 - ((NUM_SAMPLES + 2) % 3)) / 3 * 4;  // Encoded length of clip

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
float time_per_beat = (60 * 1000) / tempo;
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
int clap_state = 0; // Move 8
int fist_pump_state = 0; // Move 9
int arm_press_state = 0; // Move 9
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
const char *song_names[6] = {"Stereo Hearts", "Riptide", "Shake It Off", "Havana"}; 

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
int play_song = 0;
int song_index = 0;
int record_state = 0;
int post_state = 0;

// SD card and recording
char message_buffer[15000] = "";
int file_count = 0;
uint32_t time_since_sample;      // used for microsecond timing
const int AUDIO_IN = A0; //pin where microphone is connected
char speech_data[ENC_LEN + 200] = {0}; //global used for collecting speech data

struct Riff {
  double notes[768]; //the notes (array of doubles containing frequencies in Hz. I used https://pages.mtu.edu/~suits/notefreqs.html
  int length; //number of notes (essentially length of array.
  int rhythm_length;
  float note_period; //the timing of each note in milliseconds (take bpm, scale appropriately for note (sixteenth note would be 4 since four per quarter note) then
};

struct Lyrics {
  char title[50];
  char *lyrics[40]; // lyrics to the song
  int lyrics_len;
};

Lyrics stereo_lyrics = {"stereo", {"My heart's a stereo", "It beats for you, so listen close", "Hear my thoughts in every note", "Oh-oh", "Make me your radio", "Turn me up when you feel low", "This melody was meant for you", "Just sing along to my stereo" }, 8};
Lyrics shake_lyrics = {"shake", {"I stay out too", "late", "Got nothing in my", "brain",
"That's what people", "say, mmm-", "mmm, That's what people",
"say, mmm-", "mmm, I got on too many", "dates, ha ha", "But I can't make them", 
"stay,", "At least that's what people", "say, mmm-", "mmm, That's what people",
"say, mmm-", "mmm, But I keep", "cruising,", "Can't stop won't stop", "moving, it's", 
"like I got this", "music", "In my mind, saying", "It's gonna be alright", 
"'Cause the", "Players gonna play, play", "play, play, play, and the",
"haters gonna hate, hate,", "hate, hate, hate, Baby", "I'm just gonna shake, shake,",
"shake, shake, shake, I", "shake if off, I shake it", "off"}, 33};
Lyrics havana_lyrics = {"havana", {"Havana", "ooh na- na, ay,", "Half of my heart is in", "Havana ooh na na, ay", 
"He took my back to East", "Atlanta, na- na- na, oh,", "but my heart is in Havana,",
"There's something 'bout his", "manners, Havana ooh na na"}, 10};
Lyrics riptide_lyrics = {"riptide", {"I was scared of dentists and the", "dark,", "I was scared of pretty girls, and",
"starting conversations,", "Oh all my friends are turning", "green, You're the", 
"magician's assistant in their dreams", "Oh", "ooo-oooo-", "ooooh", "Oo-ooh, and", 
"They come unstuck", "Lady, running down to the", "riptide, taken away to the", 
"Dark side, I wanna be your left", "hand man. I", "love you, when you're singing that", 
"song, and, I got a lump in my", "throat 'cause, You're gonna sing the words", "wrong."}, 21};

int lyric_index = 0;
float karaoke_score = 0.00;
char song_title[50] = "";

struct Choreo {
  int steps; // number of steps in choreo
  int moves[20]; // sequence of dance moves
  int timing[20]; // number of beats per move 
  int counts[20]; // number of iterations of each move
};

Choreo riptide_dance = {
  7, {1, 2, 3, 4, 5, 6, 7}, {16, 16, 16, 16, 16, 32, 23}, {8, 8, 8, 16, 8, 12, 16}
};

Choreo stereo_dance = {
  20, {3, 1, 3, 2, 6, 5, 6, 7, 4, 8, 2, 6, 2, 9, 3, 1, 6, 5, 4, 10}, {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}, {4, 4, 4, 4, 4, 8, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 4}
};

Choreo havana_dance = {
  14, {4, 2, 6, 9, 6, 4, 2, 6, 9, 1, 3, 10, 5, 7}, {3, 8, 8, 8, 4, 4, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}, {3, 4, 4, 8, 2, 4, 4, 4, 8, 4, 4, 4, 8, 4}
};

Choreo shake_dance = {
  7, {8, 1, 5, 6, 2, 7, 9}, {26, 16, 16, 16, 16, 16, 16}, {12, 4, 8, 4, 4, 4, 4}
};

uint32_t song_timer;

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
622.25, 698.46, 698.46, 622.25, 622.25, 698.46, 622.25, 622.25, 622.25, 698.46, 698.46, 698.46, 698.46, 0, 0, 0, 0}, 556, 340, 150.00};

Riff stereo = {{0,0,0,0,880.0,880.0,880.0,880.0,830.61,830.61,739.99,659.25,659.25,587.33,587.33,554.37,554.37,554.37,554.37,440,659.25,659.25,739.99,659.25,659.25,587.33,587.33,554.37,554.37,493.88,493.88,554.37,554.37,554.37,554.37,440,659.25,659.25,739.99,659.25,659.25,587.33,587.33,554.37,554.37,493.88,493.88,554.37,554.37,554.37,554.37,554.37,659.25,659.25,493.88,493.88,493.88,493.88,493.88,493.88,0,0,0,0,0,0,0,0,880.0,880.0,880.0,880.0,830.61,830.61,739.99,659.25,659.25,587.33,587.33,554.37,554.37,554.37,554.37,440,659.25,659.25,739.99,659.25,659.25,587.33,587.33,554.37,554.37,493.88,493.88,554.37,554.37,554.37,554.37,440,659.25,659.25,739.99,659.25,659.25,587.33,587.33,554.37,554.37,493.88,493.88,554.37,554.37,554.37,554.37,440,659.25,739.99,739.99,659.25,659.25,659.25,554.37,0,554.37,493.88,440,440,0,0,0,0,880.0,880.0,880.0,880.0,830.61,830.61,739.99,659.25,659.25,587.33,587.33,554.37,554.37,554.37,554.37,440,659.25,659.25,739.99,659.25,659.25,587.33,587.33,554.37,554.37,493.88,493.88,554.37,554.37,554.37,554.37,440,659.25,659.25,739.99,659.25,659.25,587.33,587.33,554.37,554.37,493.88,493.88,554.37,554.37,554.37,554.37,554.37,659.25,659.25,493.88,493.88,493.88,493.88,493.88,493.88,0,0,0,0,0,0,0,0,880.0,880.0,880.0,880.0,830.61,830.61,739.99,659.25,659.25,587.33,587.33,554.37,554.37,554.37,554.37,440,659.25,659.25,739.99,659.25,659.25,587.33,587.33,554.37,554.37,493.88,493.88,554.37,554.37,554.37,554.37,440,659.25,659.25,739.99,659.25,659.25,587.33,587.33,554.37,554.37,493.88,493.88,554.37,554.37,554.37,554.37,440,659.25,739.99,739.99,659.25,659.25,659.25,554.37,0,554.37,493.88,440,440,440,0,0,0,659.25,0,440,0,554.37,0,659.25,0,0,0,415.3,0,0,0,0,0,659.25,0,554.37,0,0,0,440,0,440,493.88,440,440,440,0,0,0,659.25,0,440,0,554.37,0,659.25,0,0,0,0,0,0,0,0,440,659.25,739.99,739.99,659.25,659.25,659.25,554.37,0,554.37,493.88,440,440,0,0,554.37,0,554.37,0,554.37,0,554.37,554.37,493.88,493.88,440,440,493.88,493.88,554.37,554.37,587.33,587.33,554.37,554.37,493.88,493.88,493.88,493.88,493.88,493.88,493.88,493.88,0,0,0,0,554.37,0,554.37,0,554.37,0,554.37,554.37,493.88,493.88,440,440,493.88,493.88,554.37,554.37,587.33,587.33,554.37,554.37,493.88,493.88,493.88,493.88,493.88,493.88,493.88,493.88,0,0,554.37,0,554.37,0,554.37,0,554.37,554.37,493.88,493.88,440,440,493.88,493.88,554.37,554.37,587.33,587.33,659.25,0,659.25,659.25,659.25,659.25,659.25,659.25,659.25,659.25,659.25,659.25,0,0,0,0,739.99,0,739.99,739.99,659.25,659.25,739.99,739.99,659.25,659.25,739.99,739.99,659.25,659.25,739.99,739.99,659.25,659.25,739.99,739.99,880.0,880.0,880.0,880.0,880.0,880.0,739.99,0,739.99,0,0,0,0,0,880.0,880.0,880.0,880.0,830.61,830.61,739.99,659.25,659.25,587.33,587.33,554.37,554.37,554.37,554.37,440,659.25,659.25,739.99,659.25,659.25,587.33,587.33,554.37,554.37,493.88,493.88,554.37,554.37,554.37,554.37,440,659.25,659.25,739.99,659.25,659.25,587.33,587.33,554.37,554.37,493.88,493.88,554.37,554.37,554.37,554.37,554.37,659.25,659.25,493.88,493.88,493.88,493.88,493.88,493.88,0,0,0,0,0,0,0,0,880.0,880.0,880.0,880.0,830.61,830.61,739.99,659.25,659.25,587.33,587.33,554.37,554.37,554.37,554.37,440,659.25,659.25,739.99,659.25,659.25,587.33,587.33,554.37,554.37,493.88,493.88,554.37,554.37,554.37,554.37,440,659.25,659.25,739.99,659.25,659.25,587.33,587.33,554.37,554.37,493.88,493.88,554.37,554.37,554.37,554.37,440,659.25,739.99,739.99,659.25,659.25,659.25,554.37,0,554.37,493.88,440,440,440,0,0,0,659.25,0,440,0,554.37,0,659.25,0,0,0,415.3,0,0,0,0,0,659.25,0,554.37,0,0,0,440,0,440,493.88,440,440,440,0,0,0,659.25,0,440,0,554.37,0,659.25,0,0,0,0,0,0,0,0,440,659.25,739.99,739.99,659.25,659.25,659.25,554.37,0,554.37,493.88,493.88,493.88,440,440,440,440,0,0,0,0,440,0,0,0,0,0,0,0},
                656, 128, 166.67};
                
Riff shake = {{0,0,0,0,0,0,0,0,1174.66,1174.66,1046.5,1046.5,880.0,880.0,783.99,783.99,880.0,987.77,987.77,987.77,987.77,987.77,987.77,987.77,0,0,0,0,0,0,0,0,0,0,0,0,0,0,587.33,587.33,1174.66,1174.66,987.77,987.77,880.0,880.0,783.99,783.99,880.0,987.77,987.77,987.77,987.77,987.77,987.77,987.77,987.77,987.77,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1174.66,1174.66,987.77,987.77,880.0,880.0,783.99,783.99,987.77,987.77,880.0,880.0,783.99,783.99,783.99,783.99,0,0,0,0,587.33,659.25,659.25,659.25,587.33,587.33,587.33,587.33,0,0,0,0,1174.66,1174.66,987.77,987.77,880.0,880.0,783.99,783.99,987.77,987.77,880.0,880.0,783.99,783.99,783.99,783.99,0,0,0,0,587.33,659.25,659.25,659.25,587.33,587.33,587.33,0,587.33,0,587.33,0,1174.66,1174.66,987.77,987.77,880.0,880.0,783.99,783.99,880.0,987.77,987.77,987.77,987.77,987.77,659.25,659.25,0,0,523.25,587.33,0,0,0,0,0,0,0,0,0,0,587.33,587.33,1174.66,1174.66,987.77,987.77,880.0,880.0,783.99,783.99,880.0,987.77,987.77,987.77,987.77,987.77,987.77,987.77,0,0,0,0,0,0,0,0,0,0,587.33,0,587.33,587.33,587.33,587.33,1174.66,1174.66,987.77,987.77,880.0,880.0,783.99,783.99,987.77,987.77,880.0,880.0,783.99,783.99,783.99,783.99,0,0,0,0,587.33,659.25,659.25,659.25,587.33,587.33,587.33,587.33,0,0,0,0,1174.66,1174.66,987.77,987.77,880.0,880.0,783.99,783.99,987.77,987.77,880.0,880.0,783.99,783.99,783.99,783.99,0,0,0,0,587.33,659.25,659.25,659.25,587.33,587.33,587.33,587.33,880.0,987.77,987.77,987.77,880.0,880.0,880.0,0,783.99,783.99,783.99,0,783.99,880.0,880.0,880.0,880.0,880.0,659.25,0,659.25,659.25,659.25,659.25,0,0,0,0,880.0,880.0,880.0,0,880.0,987.77,987.77,987.77,880.0,880.0,880.0,880.0,783.99,783.99,783.99,783.99,783.99,880.0,880.0,880.0,880.0,880.0,659.25,0,659.25,659.25,659.25,659.25,0,0,659.25,659.25,880.0,880.0,880.0,0,880.0,987.77,987.77,987.77,880.0,880.0,880.0,880.0,783.99,783.99,783.99,783.99,783.99,880.0,880.0,880.0,880.0,880.0,783.99,0,783.99,783.99,783.99,783.99,0,0,0,0,880.0,880.0,880.0,0,880.0,987.77,987.77,987.77,783.99,783.99,783.99,783.99,659.25,659.25,783.99,783.99,880.0,880.0,880.0,0,880.0,880.0,987.77,987.77,880.0,880.0,783.99,783.99,783.99,783.99,659.25,783.99,783.99,783.99,783.99,783.99,659.25,659.25,0,0,0,0,0,0,659.25,659.25,783.99,783.99,880.0,0,880.0,0,880.0,0,987.77,0,783.99,783.99,783.99,783.99,587.33,659.25,659.25,659.25,587.33,587.33,587.33,587.33,440,493.88,493.88,493.88,440,440,392.0,0,392.0,0,392.0,0,880.0,0,880.0,0,880.0,0,987.77,0,783.99,783.99,783.99,783.99,587.33,659.25,659.25,659.25,587.33,587.33,587.33,587.33,440,493.88,493.88,493.88,440,440,392.0,0,392.0,0,392.0,0,880.0,0,880.0,0,880.0,0,987.77,0,783.99,783.99,783.99,783.99,587.33,659.25,659.25,659.25,587.33,587.33,587.33,587.33,440,493.88,493.88,493.88,440,440,392.0,392.0,0,0,698.46,698.46,880.0,880.0,987.77,987.77,783.99,783.99,783.99,783.99,0,0,783.99,783.99,880.0,880.0,987.77,987.77,783.99,783.99,783.99,783.99,0,0,0,0,0,0,0,0,783.99,783.99,783.99,783.99,880.0,0,880.0,0,880.0,0,987.77,0,783.99,783.99,783.99,783.99,587.33,659.25,659.25,659.25,587.33,587.33,587.33,587.33,440,493.88,493.88,493.88,440,440,392.0,0,392.0,0,392.0,0,880.0,0,880.0,0,880.0,0,987.77,0,783.99,783.99,783.99,783.99,587.33,659.25,659.25,659.25,587.33,587.33,587.33,587.33,440,493.88,493.88,493.88,440,440,392.0,0,392.0,0,392.0,0,880.0,0,880.0,0,880.0,0,987.77,0,783.99,783.99,783.99,783.99,587.33,659.25,659.25,659.25,587.33,587.33,587.33,587.33,440,493.88,493.88,493.88,440,440,392.0,392.0,0,0,698.46,698.46,880.0,880.0,987.77,987.77,783.99,783.99,783.99,783.99,0,0,783.99,783.99,880.0,880.0,987.77,987.77,783.99,783.99,783.99,783.99,0,0,0,0,0,0,0,0,0,0,0,0},
              656, 394, 93.4026};

Riff havana = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,466.16,466.16,587.33,0,587.33,0,466.16,0,466.16,0,392.0,0,0,0,0,0,587.33,0,0,0,587.33,587.33,523.25,523.25,587.33,587.33,622.25,622.25,587.33,587.33,523.25,523.25,466.16,466.16,587.33,0,587.33,0,466.16,0,466.16,0,392.0,0,0,0,0,0,587.33,0,0,0,587.33,587.33,523.25,523.25,587.33,587.33,622.25,622.25,587.33,0,587.33,523.25,466.16,466.16,587.33,0,587.33,0,466.16,0,466.16,0,392.0,0,0,0,0,0,392.0,0,0,0,587.33,587.33,523.25,523.25,587.33,587.33,622.25,622.25,587.33,587.33,523.25,523.25,466.16,466.16,587.33,587.33,466.16,466.16,0,0,587.33,587.33,523.25,523.25,587.33,587.33,523.25,523.25,466.16,466.16,587.33,587.33,440,440,587.33,587.33,466.16,466.16,440,440,466.16,466.16,440,440,466.16,466.16,783.99,0,783.99,0,587.33,0,587.33,0,622.25,622.25,587.33,587.33,523.25,523.25,587.33,587.33,0,0,587.33,0,0,0,587.33,587.33,523.25,523.25,587.33,587.33,523.25,523.25,466.16,523.25,783.99,0,783.99,0,587.33,0,587.33,0,622.25,622.25,587.33,587.33,523.25,523.25,587.33,587.33,0,0,587.33,0,0,0,587.33,587.33,523.25,523.25,587.33,587.33,523.25,523.25,466.16,523.25,783.99,0,783.99,0,783.99,783.99,880.0,880.0,932.33,932.33,880.0,880.0,783.99,783.99,880.0,880.0,0,0,587.33,0,0,0,587.33,587.33,523.25,523.25,587.33,587.33,523.25,523.25,466.16,523.25,783.99,0,783.99,0,587.33,0,587.33,0,622.25,622.25,587.33,587.33,523.25,523.25,587.33,587.33,0,0,392.0,392.0,0,0,587.33,587.33,523.25,523.25,587.33,587.33,523.25,392.0,392.0,392.0,698.46,698.46,783.99,783.99,783.99,783.99,698.46,698.46,783.99,783.99,1174.66,1174.66,1046.5,1046.5,932.33,932.33,1046.5,1046.5,880.0,880.0,440,0,880.0,0,880.0,0,880.0,0,880.0,0,880.0,0,932.33,932.33,783.99,783.99,587.33,587.33,783.99,783.99,698.46,0,698.46,0,698.46,0,698.46,0,698.46,698.46,587.33,587.33,0,0,0,0,0,0,698.46,698.46,783.99,0,783.99,783.99,698.46,698.46,783.99,783.99,783.99,783.99,698.46,698.46,783.99,783.99,1174.66,1174.66,1046.5,1046.5,932.33,932.33,1046.5,1046.5,880.0,880.0,0,0,880.0,0,880.0,0,880.0,0,880.0,0,880.0,0,932.33,932.33,783.99,783.99,0,0,932.33,932.33,783.99,783.99,783.99,783.99,932.33,932.33,880.0,880.0,0,0,932.33,0,932.33,0,932.33,0,932.33,0,932.33,0,932.33,0,0,0},
                400, 400, 142.857};

Riff long_song_to_play = {{}, 656, 166.67};
Riff stereo_long = {{}, 656, 166.67};
Riff riptide_long = {{}, 656, 150};

Riff song_to_play = stereo;
Choreo dance_to_play = stereo_dance;
Lyrics song_to_sing = stereo_lyrics;

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


//Code for sending POST request at the end

char network[] = "MIT";  //SSID for 6.08 Lab
char password[] = ""; //Password for 6.08 Lab

char host[] = "host_server";
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
    int start_ind;     // which index in the song to hit the Note at
    bool active;       // true if Note is onscreen, false otherwise
    TFT_eSPI* local_tft;
    int NOTE_CLR;
    int BKGND_CLR;
    int RADIUS;
    int DT;
    float y;          // position, velocity, acceleration for Note movement
    float v;
    float g;
    Note(TFT_eSPI* tft_to_use=&tft, uint8_t a=0, int start=0, int rad = 6, int dt=LOOP_PERIOD, 
        int note_color = TFT_BLUE, int backgnd_color = TFT_BLACK) {
          
        action = a;
        start_ind = start;
        active = true;
        local_tft = tft_to_use;
        RADIUS = rad;
        NOTE_CLR = note_color;
        BKGND_CLR = backgnd_color;
        DT = dt;
        y = -RADIUS;
        v = 60;
        g = 50;
    }
    void step() {
      if (active) {         // if Note is active: draw over previous position, calculate new position and draw there
        local_tft->drawCircle(25*(action+1), y, RADIUS, BKGND_CLR);
        v += 0.001*float(DT)*g;
        y += v*0.001*float(DT);
        local_tft->drawCircle(25*(action+1), y, RADIUS, NOTE_CLR);
      } 

    }
    void checkHit() {           // check if active Note has been hit. if so, calculate score and deactivate it
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
  int actions[175]; // which button to press (0, 1, 2, 3)
  int indices[175]; // index of the note in the Riff that this button should be pressed on
  int num_notes; // total number of notes player should hit
};

Beatmap stereo_map = {{ 0,  1,  3,  2,  3,  2,  1,  0,  1,  2,  0,  1,  2,  3,  1,  0,  1,  3,  2,  3,  2,  1,  0,  1,  2},
                      {13, 13, 20, 22, 23, 25, 27, 29, 36, 36, 47, 47, 47, 52, 54, 77, 84, 86, 87, 89, 91,100,111,111,111}, 25};

Beatmap riptide_map = {{ 0,  2,  3,  1,  1,  0,  1,  3,  2,  3,  2,  3,  0,  1,  2,  2,  3,  2,  3,  0,  1,  0,  1,  0,  1,  0,  1,  0,  1,
                         1,  2,  3,  0,  0,  1,  2,  1,  2,  3,  0,  2,  3,  1,  1,  2,  3,  0,  1,  2,  0,  3,  1,  2,  0,  3,  0,  2,  2,
                         3,  0,  3,  2,  2,  0,  1,  0,  1,  3,  3,  2,  0,  0,  3,  2,  1,  0,  3,  1,  2,  2,  3,  2,  0,  1,  0,  3,  2,
                         0,  1,  2,  0,  2,  3,  0,  2,  3,  1,  1,  0,  2,  3,  2,  1,  0,  1,  3,  0,  1},
                       {32, 36, 36, 38, 39, 42, 44, 48, 50, 52, 54, 56, 59, 59, 59, 74, 76, 78, 80, 94, 95, 96, 97, 99,101,103,105,107,109,
                       111,113,113,126,128,128,128,136,136,136,144,144,144,158,160,160,160,168,168,168,176,179,182,185,200,203,205,211,213,
                       216,219,221,227,232,234,235,239,239,243,245,246,254,256,259,264,266,271,271,275,275,280,282,283,285,286,288,291,296,
                       299,301,302,304,304,304,352,355,357,360,363,365,365,387,389,392,395,397,397,416,416},
                        90};

Beatmap shake_map = {{ 0,  3,  1,  2,  0,  3,  0,  3,  1,  2,  3,  0,  1,  0,  0,  3,  1,  2,  3,  0,  1,  0,  3,  1,  2,  0,  2,  1,
                       2,  0,  2,  2,  0,  3,  0,  0,  2,  2,  3,  2,  1,  0,  1,  2,  3,  2,  2,  3,  2,  1,  2,  3,  0,  1,  1,  3,
                       2,  1,  0,  1,  1,  1,  3,  2,  1,  0,  1,  2,  2,  2,  3,  1,  0,  1,  2,  0,  0,  3,  1,  2,  0,  2,  3,  3,
                       1,  3,  0,  1,},
                     {17, 17, 40, 40, 49, 49, 72, 78, 80, 82, 84, 92, 93, 97,104,110,112,114,116,124,125,128,136,138,140,142,150,154,
                     155,166,168,172,177,177,194,196,200,204,208,210,212,220,220,224,224,232,236,240,242,244,252,252,256,256,261,264,
                     268,272,273,278,280,288,293,296,300,305,305,310,312,320,325,328,332,337,337,342,344,352,357,360,364,366,368,372,
                     374,376,383,383},
                        88};

Beatmap havana_map = {{ 2,  3,  0,  0,  1,  1,  0,  3,  3,  0,  1,  2,  1,  0,  1,  0,  0,  1,  1,  0,  0,  3,  0,  3,  1,  2,  0,  1,
                        0,  1,  1,  2,  1,  2,  2,  3,  2,  3,  0,  3,  1,  2,  1,  3,  0,  3,  0,  3,  0,  3,  0,  1,  2,  3,  1,  2,
                        1,  2,  0,  0,  1,  1,  3,  3,  2,  2,  2,  2,  1,  2,  1,  0,  1,  3,  3,  1,  1,  2,  1,  0,  1,  2,  0,  2,
                        0,  1,  1,  1,  2,  3,  2,  1,  2,  0,  0,  1,  3,  0,  1,  2,  1,  2,  3,  0,  1,  1,  2,  3,  1,  1,  0,  3,
                        1,  2,  0,  3,  0,  2,  0,  1,  3,  3,  3,  3,  3,  1,  1,  1,  1,  1,  0,  0,  2,  3,  1,  3,  1,  2,  0,  3,
                        2,  2,  2,  2,  2,  2,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0},
                      {14, 14, 16, 18, 20, 22, 24, 30, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 62, 62, 66, 66, 70, 71, 74, 74,
                       76, 76, 80, 80, 82, 82, 84, 84, 86, 86, 98,102,104,106,110,110,114,114,118,120,122,124,128,128,128,130,134,136,
                      138,140,144,146,148,150,152,154,156,158,162,166,168,170,172,174,175,176,178,180,182,184,186,188,190,198,200,202,
                      204,208,210,212,214,216,218,220,222,226,230,234,234,240,240,240,244,244,244,248,248,252,252,254,262,266,274,274,
                      278,280,282,282,286,286,290,290,294,296,298,300,302,312,314,316,318,320,332,334,338,338,344,344,346,346,352,352,
                      354,358,360,362,364,366,368,370,376,376,380,386,388,390,392,394,396},
                      157};

Beatmap map_to_play = stereo_map;
Note map_notes[175];                

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
   //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 
    
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

//  SPIClass spi = SPIClass(VSPI);
//  spi.begin(14, 19, 13, 5); // SCK, MISO, MOSI, CS

  if(!SD.begin()){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
//
  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }
//
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

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(BUTTON4, INPUT_PULLUP);
  pinMode(AUDIO_TRANSDUCER, OUTPUT);
  pinMode(LCD_CONTROL, OUTPUT);
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);

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
}

void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
  int bv = button.update();
  int b2 = button2.update();
  read_accel();


  if (!username_selected) {
    get_username(b2);
  }
  
  if (username_selected && init_screen) {
    draw_home_screen();
    init_screen = false;
  }
  
  if (username_selected && home_screen) {
    select_game(bv); 
  }

  if (end_screen) {
    finish_game(bv);
  }

  if (begin_dance && selected_game == 1) {
    play_just_dance();
  }

  if (begin_rhythm && selected_game == 2) {
    play_rhythm_game();
  }

  if (begin_karaoke && selected_game == 3) {
    play_karaoke_game();
  }
 
  if (game_end) { // POST state
    Serial.println("you made it to the end!");
    char thing[500];
    float percentage;
    if (selected_game == 1) {
      percentage = float(just_dance_total) / (5 * dance_to_play.steps);
      sprintf(thing, "user=%s&justdance=%f&rhythm=0&karaoke=0", user, percentage);
    } else if (selected_game == 2) {
      percentage = float(rhythm_score) / (100 * map_to_play.num_notes);
      sprintf(thing, "user=%s&justdance=0&rhythm=%f&karaoke=0", user, percentage);
      rhythm_score = 0;
    } else if (selected_game == 3) {
      sprintf(thing, "user=%s&justdance=0&rhythm=0&karaoke=%f", user, karaoke_score);
    }
    post_score(thing);
    game_end = false;
    reset_dance_states();
    new_move = true;
  }
  delay(10);
}
}

/*
 * Gameplay for karaoke (recording audio portion) handled on second core
*/
void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {
  int song_id = song_to_sing.lyrics_len;
  if (record_state == 1) { // recording state
    timer = millis();
    file_count = 0;
    // do the timed recording here
    Serial.println(song_to_sing.lyrics_len);
    while (millis() - timer < song_to_play.note_period * song_to_sing.lyrics_len * 16) {
      record_audio(&file_count);
      Serial.print("File index:");
      Serial.println(file_count);
    }
    listDir(SD, "/", 0);
    record_state = 2; // switch to send state
  } else if (record_state == 2) {
    Serial.print("File count: ");
    Serial.println(file_count);
    Serial.print("song to sing lyrics len: ");
    Serial.println(song_to_sing.lyrics_len);
    for (int i = 0; i < file_count; i++) {
      // send each of the stored encoded files on the SD card to the server
      char file_name[50] = "";
      sprintf(file_name, "/audio%i.txt", i);
      readFile(SD, file_name);
      int message_len = strlen(message_buffer);
      message_len = message_len + 16;
      post_audio(message_buffer, message_len);
      Serial.println(song_to_sing.lyrics_len);
      Serial.print("placeholder val");
      Serial.println(song_id);
      tft.fillRect(11, SCREEN_HEIGHT-69, i * (128-20) / file_count, 18, GREEN);
      delay(50);
    } 
    Serial.println(song_to_sing.lyrics_len);
    if (song_id == 8) get_fft("test", &karaoke_score, "stereo");
    else if (song_id == 10) get_fft("test", &karaoke_score, "havana");
    else if (song_id == 21) get_fft("test", &karaoke_score, "riptide");
    else if (song_id == 33) get_fft("test", &karaoke_score, "shake");
    Serial.println(karaoke_score);
    record_state = 3; // set back to idle state
  } else if (record_state == 3) {
    karaoke_end();
    game_end = true;
    record_state = 0;
  }
  delay(50);
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
  sprintf(request, "POST scoreboard_url HTTP/1.1\r\n");
  sprintf(request + strlen(request), "Host: %s\r\n", host);
  strcat(request, "Content-Type: application/x-www-form-urlencoded\r\n");
  sprintf(request + strlen(request), "Content-Length: %d\r\n\r\n", strlen(thing));
  strcat(request, thing);
  Serial.println(request);
  do_http_request("host_server", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
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
  punch_state = 0; // Move 1
  hand_roll_state = 0; // Move 2
  wave_state = 0; // Move 3
  bounce_state = 0; // Move 4
  sprinkler_state = 0; // Move 5
  arm_cross_state = 0; // Move 6
  disco_state = 0; // Move 7
  clap_state = 0; // Move 8
  fist_pump_state = 0; // Move 9
  arm_press_state = 0; // Move 9
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

/*
 * Drives gameplay for just dance. Plays music and goes through the Choreo struct to present the appropriate dance move.
 * Handles appropriate state transitions upon song end.
 */
void play_just_dance() {
   if (step_num < dance_to_play.steps && millis() - song_timer > dance_time) {
      int result = similarity_score(dance_to_play.counts[step_num], move_iter);
      Serial.println(step_num);
      new_move = true;
      step_num += 1;
      dance_time += time_per_beat * dance_to_play.timing[step_num];
      add_stars(result);
      just_dance_total += result; // add to running total score
      move_iter = 0;
      tft.fillRect(0, 0, 128, 20, BLACK);
    } else if (step_num == dance_to_play.steps) { // reset all values, enter game end state
      Serial.println("End dance!");
      game_end = true;
      new_move = true;
      begin_dance = false;
      begin_rhythm = false;
      begin_karaoke = false;
      step_num = 0;
      song_state = 0; 
      song_index = 0;
      ledcWriteTone(AUDIO_PWM, 0);
      just_dance_end();
      return;
    }

    // music playing section
    if (song_index == 0) {
      Serial.println("Start music!");
      ledcWriteTone(AUDIO_PWM, song_to_play.notes[song_index]);
      song_index++;
    } 
    if (millis() - song_timer >= song_index * song_to_play.note_period) { // time to switch to the next note
      if (song_index == song_to_play.length) { // end of song
        Serial.println("End music!");
        ledcWriteTone(AUDIO_PWM, 0);
        song_state = 0; 
        song_index = 0;
      } else if (song_to_play.notes[song_index] != song_to_play.notes[song_index-1]) { // note change
        ledcWriteTone(AUDIO_PWM, song_to_play.notes[song_index]);
        song_index ++;
      } else song_index++; // otherwise increment index 
    }

    if (end_screen) {
      ledcWriteTone(AUDIO_PWM, 0);
    }

    Serial.println(step_num);
    if (dance_to_play.moves[step_num] == 1) {
      if (new_move) {
        reset_dance_states();
        sprintf(individual_scores, "%s%s", individual_scores, "Punch");
        Serial.println(individual_scores);
        new_move = false;
      }
      punch();
    } else if (dance_to_play.moves[step_num] == 2) {
      if (new_move) {
        reset_dance_states();
        sprintf(individual_scores, "%s%s", individual_scores, "Hand Roll");
        Serial.println(individual_scores);
        new_move = false;
      }
      hand_roll();
    } else if (dance_to_play.moves[step_num] == 3) {
      if (new_move) {
        reset_dance_states();
        sprintf(individual_scores, "%s%s", individual_scores, "Wave");
        Serial.println(individual_scores);
        new_move = false;
      }
      wave();
    } else if (dance_to_play.moves[step_num] == 4) {
      if (new_move) {
        reset_dance_states();
        sprintf(individual_scores, "%s%s", individual_scores, "Bounce");
        Serial.println(individual_scores);
        new_move = false;
      }
      bounce();
    } else if (dance_to_play.moves[step_num] == 5) {
      if (new_move) {
        reset_dance_states();
        sprintf(individual_scores, "%s%s", individual_scores, "Sprinkler");
        Serial.println(individual_scores);
        new_move = false;
      }
      sprinkler();
    } else if (dance_to_play.moves[step_num] == 6) {
      if (new_move) {
        reset_dance_states();
        sprintf(individual_scores, "%s%s", individual_scores, "Arm Cross");
        Serial.println(individual_scores);
        new_move = false;
      }
      arm_cross();
    } else if (dance_to_play.moves[step_num] == 7) {
      if (new_move) {
        reset_dance_states();
        sprintf(individual_scores, "%s%s", individual_scores, "Disco");
        Serial.println(individual_scores);
        new_move = false;
      }
      disco();
    } else if (dance_to_play.moves[step_num] == 8) {
      if (new_move) {
        reset_dance_states();
        sprintf(individual_scores, "%s%s", individual_scores, "Clap");
        Serial.println(individual_scores);
        new_move = false;
      }
      clap();
    } else if (dance_to_play.moves[step_num] == 9) {
      if (new_move) {
        reset_dance_states();
        sprintf(individual_scores, "%s%s", individual_scores, "Fist Pump");
        Serial.println(individual_scores);
        new_move = false;
      }
      fist_pump();
    } else if (dance_to_play.moves[step_num] == 10) {
      if (new_move) {
        reset_dance_states();
        sprintf(individual_scores, "%s%s", individual_scores, "Arm Press");
        Serial.println(individual_scores);
        new_move = false;
      }
      arm_press();
    }
}

/*
 * Part of gameplay for karaoke handled on first core. 
 * Plays audio while displaying lyrics and handles appropriate state transitions.
 */
void play_karaoke_game() {
    if (lyric_index > song_to_sing.lyrics_len) { // reset all values, enter game end state
      Serial.println("End singing!");
      song_state = 0;
      new_move = true;
      begin_dance = false;
      begin_rhythm = false;
      begin_karaoke = false;
      step_num = 0;
      song_state = 0; 
      song_index = 0;
      lyric_index = 0;
      ledcWriteTone(AUDIO_PWM, 0);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(25, 30, 2);
      tft.println("Getting your");
      tft.setCursor(45, 48, 2);
      tft.println("score...");
      tft.drawRect(10, SCREEN_HEIGHT-70, 108, 20, WHITE);
    }
      else if (song_index == 0) {
      Serial.println("Start music!");
      ledcWriteTone(AUDIO_PWM, song_to_play.notes[song_index]);
      Serial.println(song_index);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 2);
      tft.println(song_to_sing.lyrics[lyric_index]);
      lyric_index ++;
      song_index++;
      } 
      else if (millis() - song_timer >= song_index * song_to_play.note_period) { // time to switch to the next note
        if ((song_index)%16 == 0) {
          Serial.println(song_index);
          // start a new screen every set
          if (lyric_index % 2 == 0) {
            tft.fillScreen(TFT_BLACK);
            tft.setCursor(0, 0, 2);
          }
          tft.println(song_to_sing.lyrics[lyric_index]);
          if (lyric_index <= song_to_sing.lyrics_len) {
            lyric_index++;
          }
        }
        if (song_index == song_to_play.length) { // end of song
          ledcWriteTone(AUDIO_PWM, 0);
          song_state = 0; 
          song_index = 0;
          game_end = true;
        } else if (song_to_play.notes[song_index] != song_to_play.notes[song_index-1]) { // note change
          ledcWriteTone(AUDIO_PWM, song_to_play.notes[song_index]);
          song_index ++;
        } else song_index++; // otherwise increment index 
      }
}


/*
 * Drives gameplay for rhythm game. Goes through array of Notes for current songs and implements actions (falling, being hit) 
 * for the ones that are currently active.
 * Plays audio frequencies for the song and handles appropriate state transitions once the song/Riff reaches its end.
 */
void play_rhythm_game() {

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
      if (song_index == song_to_play.rhythm_length && begin_rhythm) { // end of song
        ledcWriteTone(AUDIO_PWM, 0);
        song_state = 0; 
        song_index = 0;

        game_end = true;
        begin_rhythm = false;
        begin_dance = false;
        begin_karaoke = false;
        rhythm_end();        

        digitalWrite(R, LOW);
        digitalWrite(G, LOW);
        
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
