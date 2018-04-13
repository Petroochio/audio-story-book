// A mic or line-in connection is required. See page 13 of the 
// datasheet for wiring

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// define the pins used
#define RESET 9      // VS1053 reset pin (output)
#define CS 10        // VS1053 chip select pin (output)
#define DCS 8        // VS1053 Data/command select pin (output)
#define CARDCS A0     // Card chip select pin
#define DREQ A1       // VS1053 Data request, ideally an Interrupt pin

#define REC_BUTTON 2
#define PLAY_BUTTON 3
#define DEL_BUTTON 4
#define NEXT_BUTTON 7 // replace with slider and maybe analog or a bunch of digitals

#define LED_ZERO 5
#define LED_ONE 6
#define LED_TWO 7
// #define LED_THREE 7

Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(RESET, CS, DCS, DREQ, CARDCS);

File recording;  // the file we will save our recording to
#define RECBUFFSIZE 128  // 64 or 128 bytes.
uint8_t recording_buffer[RECBUFFSIZE];

// State Stuff
int WAITING = 0;
int RECORDING = 1;
int PLAYING = 2;
int ANIMATION = 3;

int currentState = WAITING;

// Recording Num
int currentFile = 0;
char filename[15];

void setup() {
  Serial.begin(9600);
  strcpy(filename, "RECORD01.OGG");
  // musicPlayer.sineTest(0x44, 500);

  if (!SD.begin(CARDCS)) {
    Serial.println("SD failed, or not present");
    while (1);  // don't do anything more
    // Add more error handling here, like flash LEDs or something
  }
  // initialise the music player
  resetPlayer();
//  File file = SD.open(filename);
//  Serial.println(file);
//  File.close
// Make a tone to indicate VS1053 is working
  
  // when the button is pressed, record!
  pinMode(REC_BUTTON, INPUT_PULLUP);
  pinMode(DEL_BUTTON, INPUT_PULLUP);
  pinMode(NEXT_BUTTON, INPUT_PULLUP);
  pinMode(PLAY_BUTTON, INPUT_PULLUP);

  // LED stuff
  pinMode(LED_ZERO, OUTPUT);
  pinMode(LED_ONE, OUTPUT);
  pinMode(LED_TWO, OUTPUT);
  // pinMode(LED_THREE, OUTPUT);
}

void resetPlayer() {
  if (!musicPlayer.begin()) {
    Serial.println("VS1053 not found");
    while (1);  // don't do anything more
    // Add more error handling here, like flash LEDs or something

    // Set volume for left, right channels. lower numbers == louder volume!
  }
  musicPlayer.setVolume(10,10);
  // load plugin from SD card! We'll use mono 44.1KHz, high quality
  if (! musicPlayer.prepareRecordOgg("v44k1q05.img")) {
     Serial.println("Couldn't load plugin!");
     while (1);    
  }

  Serial.println("done reset");
  delay(100);
  musicPlayer.sineTest(0x44, 500);
}

int prevRec = 0;
int prevPlay = 0;
int prevNext = 0;
int prevDel = 0;

void loop() {
  // New State Logic
  // make these bools?
  int isRec = digitalRead(REC_BUTTON);
  int isPlay = digitalRead(PLAY_BUTTON);
  int isNext = digitalRead(NEXT_BUTTON);
  int isDel = digitalRead(DEL_BUTTON);
  int sliderLeft = analogRead(A4);
  int sliderRight = analogRead(A5);
//  Serial.println(isDel);

  if (currentState == WAITING) {
    if (sliderLeft < 500) {
//      Serial.println(1);
        strcpy(filename, "RECORD01.OGG");
    } else if (sliderRight < 500) {
//      Serial.println(4);
        strcpy(filename, "RECORD04.OGG");
    } else if (sliderRight >= 519 && sliderLeft <= 525) {
//      Serial.println(3);
        strcpy(filename, "RECORD03.OGG");
    } else {
//      Serial.println(2);
        strcpy(filename, "RECORD02.OGG");
    }
//    delay(200);
    
    if (isRec == 0 && prevRec == 1) {
//      if (SD.exists(filename)) {
//        SD.remove(filename);
//      }
      // start rec stuff
      Serial.println("Begin recording");
      Serial.println(SD.exists(filename));
      recording = SD.open(filename, FILE_WRITE);
      Serial.println(":D");
      if (!recording) {
          Serial.println("Couldn't open file to record!");
          while (1);
      }
      Serial.println(":1");
      musicPlayer.startRecordOgg(false); // use microphone (for linein, pass in 'false')
      Serial.println(":2");
      currentState = RECORDING;
    } else if (isPlay == 0 && prevPlay == 1) {
      Serial.println("start play");
      // play stuff
      musicPlayer.begin();
      musicPlayer.setVolume(10,10);
      musicPlayer.playFullFile(filename);
      resetPlayer();
    } else if (isDel == 0 && prevDel == 1) {
      // del stuff
      // Set some sort of timer on this so it ain't so sensitive
      Serial.println("del");
      if (SD.exists(filename)) {
        SD.remove(filename);
      }
    }
  } else if (currentState == RECORDING) {
    if (isRec == 1) {
      Serial.println("End recording");
      musicPlayer.stopRecordOgg();
      currentState = WAITING;
      // flush all the data!
      boolean recState = true;
      saveRecordedData(recState); // no longer recording
      // close it up
      recording.close();
      delay(100);
      delay(100);
      delay(400);
      
    } else {
      Serial.println("isRec");
      boolean recState = true;
      saveRecordedData(recState); // still recording
    }
  }

  prevRec = isRec;
  prevPlay = isPlay;
  prevNext = isNext;
  prevDel = isDel;
}

void startRecording() {
  Serial.println("Begin recording");
  Serial.println(SD.exists(filename));
  recording = SD.open(filename, FILE_WRITE);
  
  if (! recording) {
      Serial.println("Couldn't open file to record!");
      while (1);
  }
  musicPlayer.startRecordOgg(false); // use microphone (for linein, pass in 'false')
  Serial.println(":D");
}

uint16_t saveRecordedData(boolean isrecord) {
  uint16_t written = 0;
  
    // read how many words are waiting for us
  uint16_t wordswaiting = musicPlayer.recordedWordsWaiting();
  
  // try to process 256 words (512 bytes) at a time, for best speed
  while (wordswaiting > 256) {
    //Serial.print("Waiting: "); Serial.println(wordswaiting);
    // for example 128 bytes x 4 loops = 512 bytes
    for (int x=0; x < 512/RECBUFFSIZE; x++) {
      // fill the buffer!
      for (uint16_t addr=0; addr < RECBUFFSIZE; addr+=2) {
        uint16_t t = musicPlayer.recordedReadWord();
        //Serial.println(t, HEX);
        recording_buffer[addr] = t >> 8; 
        recording_buffer[addr+1] = t;
      }
      if (! recording.write(recording_buffer, RECBUFFSIZE)) {
            Serial.print("Couldn't write "); Serial.println(RECBUFFSIZE); 
            while (1);
      }
    }
    // flush 512 bytes at a time
    recording.flush();
    written += 256;
    wordswaiting -= 256;
  }
  
  wordswaiting = musicPlayer.recordedWordsWaiting();
  if (!isrecord) {
    Serial.print(wordswaiting); Serial.println(" remaining");
    // wrapping up the recording!
    uint16_t addr = 0;
    for (int x=0; x < wordswaiting-1; x++) {
      // fill the buffer!
      uint16_t t = musicPlayer.recordedReadWord();
      recording_buffer[addr] = t >> 8; 
      recording_buffer[addr+1] = t;
      if (addr > RECBUFFSIZE) {
          if (! recording.write(recording_buffer, RECBUFFSIZE)) {
                Serial.println("Couldn't write!");
                while (1);
          }
          recording.flush();
          addr = 0;
      }
    }
    if (addr != 0) {
      if (!recording.write(recording_buffer, addr)) {
        Serial.println("Couldn't write!"); while (1);
      }
      written += addr;
    }
    musicPlayer.sciRead(VS1053_SCI_AICTRL3);
    if (! (musicPlayer.sciRead(VS1053_SCI_AICTRL3) & _BV(2))) {
       recording.write(musicPlayer.recordedReadWord() & 0xFF);
       written++;
    }
    recording.flush();
  }

  return written;
}
