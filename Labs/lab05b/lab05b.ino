#include <mpu6050_esp32.h>
#include<math.h>
#include<string.h>
#include <TFT_eSPI.h> 
#include <SPI.h>
TFT_eSPI tft = TFT_eSPI();  

// Minimalist C Structure for containing a musical "riff"
//can be used with any "type" of note. Depending on riff and pauses,
// you may need treat these as eighth or sixteenth notes or 32nd notes...sorta depends
struct Riff {
  double notes[288]; //the notes (array of doubles containing frequencies in Hz. I used https://pages.mtu.edu/~suits/notefreqs.html
  int length; //number of notes (essentially length of array.
  float note_period; //the timing of each note in milliseconds (take bpm, scale appropriately for note (sixteenth note would be 4 since four per quarter note) then
};

//Kendrick Lamar's HUMBLE
// needs 32nd notes to sound correct...song is 76 bpm, so that's 98.15 ms per 32nd note though the versions on youtube vary by a few ms so i fudged it
Riff humble = {{
    311.13, 311.13, 0 , 0, 311.13, 311.13, 0, 0,  //beat 1
    0, 0, 0, 0, 329.63, 329.63, 0, 0,             //beat 2
    311.13, 311.13, 0, 0, 207.65, 0, 207.65, 0,   //beat 3
    0, 0, 207.65, 207.65, 329.63, 329.63, 0, 0    //beat 4
  }, 32, 100.15 //32 32nd notes in measure, 108.15 ms per 32nd note
};

Riff more_humbler = {{
    311.13, 311.13, 0 , 0, 311.13, 311.13, 0, 0,  //beat 1
    0, 0, 0, 0, 329.63, 329.63, 0, 0,             //beat 2
    311.13, 311.13, 0, 0, 207.65, 0, 207.65, 0,   //beat 3
    0, 0, 207.65, 207.65, 329.63, 329.63, 0, 0,   //beat 4
    311.13, 311.13, 0 , 0, 311.13, 311.13, 0, 0,  //beat 1
    0, 0, 0, 0, 329.63, 329.63, 0, 0,             //beat 2
    311.13, 311.13, 0, 0, 207.65, 0, 207.65, 0,   //beat 3
    0, 0, 207.65, 207.65, 329.63, 329.63, 0, 0,    //beat 4
    0, 0, 0, 0, 0, 0, 415.30, 415.30, //pause
    415.30, 415.30, 0 , 0, 415.30, 415.30, 0, 0,
    415.30, 415.30, 0 , 0, 415.30, 415.30, 0, 0,
    440.00, 440.00, 0 , 0, 440.00, 440.00, 0, 0,
    311.13, 311.13, 0 , 0, 311.13, 311.13, 0, 0,
    311.13, 0, 311.13 , 0, 311.13, 0, 311.13, 0,
    311.13, 0, 311.13 , 0, 0, 0, 329.63, 329.63,
    311.13, 311.13, 0 , 0, 311.13, 311.13, 0, 0,
    311.13, 311.13, 0 , 0, 311.13, 311.13, 0, 0,
    311.13, 0, 311.13 , 0, 311.13, 0, 311.13, 0,
    311.13, 0, 311.13 , 0, 0, 0, 329.63, 329.63,
    311.13, 311.13, 0 , 0, 311.13, 311.13, 0, 0,
    311.13, 311.13, 0 , 0, 311.13, 311.13, 0, 0,  //beat 1
    0, 0, 0, 0, 329.63, 329.63, 0, 0,             //beat 2
    311.13, 311.13, 0, 0, 207.65, 0, 207.65, 0,   //beat 3
    0, 0, 207.65, 207.65, 329.63, 329.63, 0, 0,   //beat 4
    311.13, 311.13, 0 , 0, 311.13, 311.13, 0, 0,  //beat 1
    0, 0, 0, 0, 329.63, 329.63, 0, 0,             //beat 2
    311.13, 311.13, 0, 0, 207.65, 0, 207.65, 0,   //beat 3
    0, 0, 207.65, 207.65, 329.63, 329.63, 0, 0,
    110.00, 110.00, 0, 0, 110.00, 110.00, 110.00, 0
  }, 288, 100.15 //32 32nd notes in measure, 108.15 ms per 32nd note
};

//Beyonce aka Sasha Fierce's song Formation off of Lemonade. Don't have the echo effect
// needs 16th notes and two measures to sound correct...song is 123 bpm, so that's around 120.95 ms per 16th note though the versions on youtube
// vary even within the same song quite a bit, so sorta I matched to her video for the song.
Riff formation = {{
    261.63, 0, 261.63 , 0,   0, 0, 0, 0, 261.63, 0, 0, 0, 0, 0, 0, 0, //measure 1 Y'all haters corny with that illuminati messssss
    311.13, 0, 311.13 , 0,   0, 0, 0, 0, 311.13, 0, 0, 0, 0, 0, 0, 0 //measure 2 Paparazzi catch my fly and my cocky freshhhhhhh
  }, 32, 120.95 //32 32nd notes in measure, 108.15 ms per 32nd note
};

//Justin Bieber's Sorry:
// only need the 16th notes to make sound correct. 100 beats (notes) per minute in song means 150 ms per 16th note
// riff starts right at the doo doo do do do do doo part rather than the 2-ish beats leading up to it. That way you
// can go right into the good part with the song. Sorry if that's confusing.
Riff sorry = {{ 1046.50, 1244.51 , 1567.98, 0.0, 1567.98, 0.0, 1396.91, 1244.51, 1046.50, 0, 0, 0, 0, 0, 0, 0}, 16, 150};


//create a song_to_play Riff that is one of the three ones above. 
Riff song_to_play = more_humbler;  //select one of the riff songs


const uint32_t READING_PERIOD = 150; //milliseconds
double MULT = 1.059463094359; //12th root of 2 (precalculated) for note generation
double A1 = 55; //A1 55 Hz  for note generation
const uint8_t NOTE_COUNT = 97; //number of notes set at six octaves from

//buttons for today 5 is labeled 5, 3 is actually labeled RX
uint8_t BUTTON1 = 5;
uint8_t BUTTON2 = 3;
int last_update = millis();

//pins for LCD and AUDIO CONTROL
uint8_t LCD_CONTROL = 25;
uint8_t AUDIO_TRANSDUCER = 26;

//PWM Channels. The LCD will still be controlled by channel 0, we'll use channel 1 for audio generation
uint8_t LCD_PWM = 0;
uint8_t AUDIO_PWM = 1;

//arrays you need to prepopulate for use in the run_instrument() function
double note_freqs[NOTE_COUNT];
float accel_thresholds[NOTE_COUNT + 1];

//global variables to help your code remember what the last note was to prevent double-playing a note which can cause audible clicking
float new_note = 0;
float old_note = 0;

MPU6050 imu; //imu object called, appropriately, imu


void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for Serial to show up
  Wire.begin();
  delay(50); //pause to make sure comms get set up
  if (imu.setupIMU(1)) {
    Serial.println("IMU Connected!");
  } else {
    Serial.println("IMU Not Connected :/");
    Serial.println("Restarting");
    ESP.restart(); // restart the ESP (proper way)
  }
  tft.init(); //initialize the screen
  tft.setRotation(2); //set rotation for our layout
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  Serial.printf("Frequencies:\n"); //print out your frequencies as you make them to help debugging
  double note_freq = A1;
  //fill in note_freq with appropriate frequencies from 55 Hz to 55*(MULT)^{NOTE_COUNT-1} Hz
  for(int i = 0; i < NOTE_COUNT; i++) {
    Serial.println((A1*pow(MULT, i)));
    note_freqs[i] = A1*pow(MULT, i);
  }
 

  //print out your accelerometer boundaries as you make them to help debugging
  Serial.printf("Accelerometer thresholds:\n");
  //fill in accel_thresholds with appropriate accelerations from -1 to +1
  for(int i = 0; i < NOTE_COUNT+1; i++) {
    Serial.println((-1 + (2.0/NOTE_COUNT)*i));
    accel_thresholds[i] = -1 + (2.0/NOTE_COUNT)*i;
  }
  
  //start new_note as at middle A or thereabouts.
  new_note = note_freqs[NOTE_COUNT - NOTE_COUNT / 2]; //set starting note to be middle of range.

  //four pins needed: two inputs, two outputs. Set them up appropriately:
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(AUDIO_TRANSDUCER, OUTPUT);
  pinMode(LCD_CONTROL, OUTPUT);

  //set up AUDIO_PWM which we will control in this lab for music:
  ledcSetup(AUDIO_PWM, 0, 12);//12 bits of PWM precision
  ledcWrite(AUDIO_PWM, 0); //0 is a 0% duty cycle for the NFET
  ledcAttachPin(AUDIO_TRANSDUCER, AUDIO_PWM);

  //set up the LCD PWM and set it to 
  pinMode(LCD_CONTROL, OUTPUT);
  ledcSetup(LCD_PWM, 100, 12);//12 bits of PWM precision
  ledcWrite(LCD_PWM, 0); //0 is a 0% duty cycle for the PFET...increase if you'd like to dim the LCD.
  ledcAttachPin(LCD_CONTROL, LCD_PWM);


  //play A440 for testing.
  //comment OUT this line for the latter half of the lab.
  //ledcWriteTone(AUDIO_PWM, 440); //COMMENT THIS OUT AFTER VERIFYING WORKING
}

//make the accelerometer instrument:
//function can be blocking (just for this lab :))
void run_instrument() {
  imu.readAccelData(imu.accelCount);//read imu
  float x = imu.accelCount[0] * imu.aRes;
  float gs = -1.0;
  int i = 0;
  while (x > gs) {
    i++;
    gs = accel_thresholds[i];
    if (i == NOTE_COUNT-1){
      break;
    }
  }
  new_note = note_freqs[i];
  if (new_note != old_note) {
    ledcWriteTone(AUDIO_PWM, new_note);
    if (new_note > old_note) {
      tft.fillScreen(TFT_ORANGE);
    }
    else {
      tft.fillScreen(TFT_BLUE);
    }
    tft.setCursor(0,0,4);
    tft.print(new_note);
    tft.print("           ");
  }
  old_note = new_note;
}


//make the riff player. Play whatever the current riff is (specified by struct instance song_to_play)
//function can be blocking (just for this lab :) )
void play_riff(song_to_play) {
  int num_notes = song_to_play.length;
  float period = song_to_play.note_period;
  for(int note_num = 0; note_num < num_notes; note_num++) {
    new_note = song_to_play.notes[note_num];
    if (new_note != old_note)
      ledcWriteTone(AUDIO_PWM, new_note);
      tft.setCursor(0,0,4);
      tft.print(new_note);
      tft.print("           ");
    old_note = new_note;
    delay(period);
  }
}


void loop() {
  if (!digitalRead(BUTTON1)) {
    if (millis() - last_update > READING_PERIOD) {
      run_instrument();
      last_update = millis();
    }
  } else if (!digitalRead(BUTTON2)) {
    play_riff();
  }
  else {
    ledcWriteTone(AUDIO_PWM, 0);
  }
}
