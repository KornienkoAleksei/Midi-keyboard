#include <SPI.h>                                           // We connect the library to work with the SPI
#include <nRF24L01.h>                                      // We connect the settings file from the library RF24
#include <RF24.h>                                          // Connect the library to work with nRF24L01+
RF24 radio(9, 53);                                         // Create a radio object to work with the RF24 library, specifying the pin numbers nRF24L01+ (CE, CSN)
unsigned int radio_data [3];                               // Create an array to receive data

int en_A [10] = {14, 49, 48, 34, 25, 10, 18, 45, 38, 33};
int en_B [10] = {15, 20, 47, 39, 27, 12, 16, 46, 41, 31};

int bt [18] = {35, 30, 24, 22, 43, 32, 28, 26, 13, 19, 44, 37, 23}; // array of controller outputs from buttons

int i; //variable for cycles
int j;
int k;
int bt_bank;
int en_bank;
int en_prev [10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int en_0_pos[4][10] = {{63, 63, 63, 63, 63, 63, 63, 63, 63, 63}, {63, 63, 63, 63, 63, 63, 63, 63, 63, 63}, {63, 63, 63, 63, 63, 63, 63, 63, 63, 63}, {63, 63, 63, 63, 63, 63, 63, 63, 63, 63}};
int en_0_save[4][10];
int encoder_A;
int encoder_B;
int radio_state = 0;

//button statuses
int bt_prev [18];
int shift_bt_prev; 
int bt_radio_state; // radio/network mode

byte data_in [64];
int data_in_size;
int data_tr = 0;

void setup() {
  Serial.begin(38400);
  radio.begin();                                        // We initiate work nRF24L01+
  delay(500);
  radio.setAutoAck   (1) ;
  radio.setChannel(55);                                  // Specify the data transmission channel (from 0 to 127), 5 means data transmission is carried out at a frequency of 2.405 GHz (on one channel there can be only 1 receiver and up to 6 transmitters)
  radio.setDataRate     (RF24_1MBPS);                   // Specify the data transfer rate (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Mbps
  radio.setPALevel      (RF24_PA_HIGH);                 // Specify the transmitter power (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
  radio.openWritingPipe (0x1234567890LL);
  radio.openReadingPipe (1, 0x1234567890LL);
  radio.startListening  ();
  for ( i = 10; i <= 49; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  for (i = 2; i <= 8; i++) {
    pinMode(i, OUTPUT);
    digitalWrite ( i, LOW);
  }
  digitalWrite ( 3, HIGH);
}

void loop() {
  //read data block
  if (Serial.available() > 0 && radio_state == 1) {
    Serial.flush();
    data_in_size = Serial.available();
    Serial.readBytes (data_in , data_in_size);
    delay(20);
    data_tr = 1;
  }
 
  if (data_tr == 1) {
    for (i = 0; i <= data_in_size; i++) {
      if (data_in [i] == 0xB0) {
        en_0_pos[0][data_in [i + 1]] = data_in [i + 2];
      }
       if (data_in [i] == 0xB1) {
        en_0_pos[1][data_in [i + 1]] = data_in [i + 2];
      }
       if (data_in [i] == 0xB2) {
        en_0_pos[2][data_in [i + 1]] = data_in [i + 2];
      }
       if (data_in [i] == 0xB3) {
        en_0_pos[3][data_in [i + 1]] = data_in [i + 2];
      }
    }
    data_tr = 0;
 }
 
  //operation mode: wire - radio
  encoder_A = digitalRead(40);
  if (encoder_A == 1 && bt_radio_state == 0) {
    if (radio_state == 0) {
      radio_state = 1;
      digitalWrite ( 3, LOW);
      digitalWrite ( 8, HIGH); //diode 4 red
    } else if (radio_state == 1) {
      radio_state = 0;
      digitalWrite ( 8, LOW); //diode 4 green
      digitalWrite ( 3, HIGH);
    }
    bt_radio_state = 1;
    delay(20);
  }
  
  if (encoder_A == 0 &&  bt_radio_state == 1) {
    bt_radio_state = 0;
  }

  //encoder banks
  encoder_A = digitalRead(42);
  if (encoder_A == 1 && shift_bt_prev == 0) {
    if (bt_bank == 0) {
      bt_bank = 1;
      digitalWrite ( 2, LOW);
      digitalWrite ( 6, HIGH); //diode 2 green
    } else if (bt_bank == 1) {
      bt_bank = 2;
      digitalWrite ( 6, LOW); //diode 2 red
      digitalWrite ( 2, HIGH);
    } else if (bt_bank == 2) {
      bt_bank = 0;
      digitalWrite ( 2, LOW); //diode 2 off
      digitalWrite ( 6, LOW);
    }
    shift_bt_prev = 1;
    delay(20);
  }
  if (encoder_A == 0 && shift_bt_prev == 1) {
    shift_bt_prev = 0;
  }

  //setting encoder banks
  encoder_A = digitalRead(11);
  if (encoder_A == 1 && bt_prev [13] == 0) {
    if (en_bank != 0) {
      en_bank = 0;
      digitalWrite ( 5, LOW);
      digitalWrite ( 4, LOW); //diode 3 off
    }
    bt_prev [13] = 1;
    delay(20);
  }
  if (encoder_A == 0 &&  bt_prev [13] == 1) {
    bt_prev [13] = 0;
  }
  encoder_A = digitalRead(17);
  if (encoder_A == 1 && bt_prev [14] == 0) {
    if (en_bank != 1) {
      en_bank = 1;
      digitalWrite ( 5, HIGH);
      digitalWrite ( 4, LOW); //diode 3 off
    }
    bt_prev [14] = 1;
    delay(20);
  }
  if (encoder_A == 0 &&  bt_prev [14] == 1) {
    bt_prev [14] = 0;
  }
  encoder_A = digitalRead(21);
  if (encoder_A == 1 && bt_prev [15] == 0) {
    if (en_bank != 2) {
      en_bank = 2;
      digitalWrite ( 5, LOW);
      digitalWrite ( 4, HIGH); //diode 3 off
    }
    bt_prev [15] = 1;
    delay(20);
  }
  if (encoder_A == 0 &&  bt_prev [15] == 1) {
    bt_prev [15] = 0;
  }
  encoder_A = digitalRead(36);
  if (encoder_A == 1 && bt_prev [16] == 0) {
    if (en_bank != 3) {
      en_bank = 3;
      digitalWrite ( 5, HIGH);
      digitalWrite ( 4, HIGH); //diode 3 off
    }
    bt_prev [16] = 1;
    delay(20);
  }
  if (encoder_A == 0 &&  bt_prev [16] == 1) {
    bt_prev [16] = 0;
  }
  // button processing
  for (i = 0 ; i <= 12; i++) {
    encoder_A = digitalRead(bt [i] );
    if (encoder_A == 1 && bt_prev [i] == 0 ) {
      if (radio_state == 1) {
        Serial.write(0x90);
        Serial.write(i + 5 + 20 * bt_bank);
        Serial.write(127);
      }
      if (radio_state == 0) {
        radio_data [0] = 0x90;
        radio_data [1] = i + 5 + 20 * bt_bank;
        radio_data [2] = 127;
        radio.stopListening();
        radio.write(&radio_data, sizeof(radio_data));
        radio.startListening  ();
      }
      bt_prev [i] = 1;
    }
    if (encoder_A == 0 && bt_prev [i] == 1) {
      bt_prev [i] = 0;
    }
  }
  //encoder processing
  for (i = 0; i <= 9; i++) {
    encoder_A = digitalRead(en_A[i]);     // read the state of output A of the encoder
    if ( encoder_A == 0 && en_prev[i] == 1 ) {  // if the state has changed from positive to zero
      if (digitalRead(en_B[i]) == 1) {
        en_0_pos [en_bank][i] ++;
      }
      else {
        en_0_pos[en_bank][i] --;
      }
      if (radio_state == 1) {
        Serial.write(0xB0 + en_bank);
        Serial.write(i);
        Serial.write (en_0_pos[en_bank][i]);
      }
      if (radio_state == 0) {
        radio_data [0] = 0xB0 + en_bank;
        radio_data [1] = i;
        radio_data [2] = en_0_pos[en_bank][i];
        radio.stopListening();
        radio.write(&radio_data, sizeof(radio_data));
        radio.startListening  ();
      }
    }
    en_prev[i] = encoder_A;
  }
  delay(10);
}
