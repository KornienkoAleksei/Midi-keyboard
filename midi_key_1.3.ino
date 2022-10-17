#include <SPI.h>                                          // Подключаем библиотеку для работы с шиной SPI
#include <nRF24L01.h>                                     // Подключаем файл настроек из библиотеки RF24
#include <RF24.h>                                         // Подключаем библиотеку для работы с nRF24L01+
RF24           radio(9, 53);                              // Создаём объект radio для работы с библиотекой RF24, указывая номера выводов nRF24L01+ (CE, CSN)
unsigned int        radio_data [3];                                   // Создаём массив для приёма данных
//encoder________0___1___2___3___4___5___6___7___8___9
int en_A [10] = {14, 49, 48, 34, 25, 10, 18, 45, 38, 33};
int en_B [10] = {15, 20, 47, 39, 27, 12, 16, 46, 41, 31};
//button________0___1___2___3___4___5___6___7___8___9__10__11__12__13__14__15__16__17
int bt [18] = {35, 30, 24, 22, 43, 32, 28, 26, 13, 19, 44, 37, 23}; // массив выводов контроллера с кнопок


int i; //переменная для циклов
int j;
int k;
int bt_bank; // банка кнопок
int en_bank; // банки энкодеров
int en_prev [10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int en_0_pos[4][10] = {{63, 63, 63, 63, 63, 63, 63, 63, 63, 63}, {63, 63, 63, 63, 63, 63, 63, 63, 63, 63}, {63, 63, 63, 63, 63, 63, 63, 63, 63, 63}, {63, 63, 63, 63, 63, 63, 63, 63, 63, 63}};
int en_0_save[4][10];
int encoder_A;
int encoder_B;
int radio_state = 0;

//статусы кнопок
int bt_prev [18]; //кнопки
int shift_bt_prev; // банки кнопок
int bt_radio_state; // режим работы радио/сеть

byte data_in [64];
int data_in_size;
int data_tr = 0;

void setup() {
  //описание выводов
  Serial.begin(38400);
  radio.begin();                                        // Инициируем работу nRF24L01+
  delay(500);
  radio.setAutoAck   (1) ;
  radio.setChannel(55);                                  // Указываем канал передачи данных (от 0 до 127), 5 - значит передача данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
  radio.setDataRate     (RF24_1MBPS);                   // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
  radio.setPALevel      (RF24_PA_HIGH);                 // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
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
  //  Serial.println("end setup");
  digitalWrite ( 3, HIGH);
}

void loop() {


  //чтение данных блок
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
 
  //режим работы сеть - прд/прм
  encoder_A = digitalRead(40);
  if (encoder_A == 1 && bt_radio_state == 0) {
    if (radio_state == 0) {
      radio_state = 1;
      digitalWrite ( 3, LOW);
      digitalWrite ( 8, HIGH); //диод 4 красный
    } else if (radio_state == 1) {
      radio_state = 0;
      digitalWrite ( 8, LOW); //диод 4 зеленый
      digitalWrite ( 3, HIGH);
    }
    bt_radio_state = 1;
    delay(20);
  }
  
  if (encoder_A == 0 &&  bt_radio_state == 1) {
    bt_radio_state = 0;
  }

  //задание бакни кнопок на брата
  encoder_A = digitalRead(42);
  if (encoder_A == 1 && shift_bt_prev == 0) {
    if (bt_bank == 0) {
      bt_bank = 1;
      digitalWrite ( 2, LOW);
      digitalWrite ( 6, HIGH); //диод 2 зеленый
    } else if (bt_bank == 1) {
      bt_bank = 2;
      digitalWrite ( 6, LOW); //диод 2 красный
      digitalWrite ( 2, HIGH);
    } else if (bt_bank == 2) {
      bt_bank = 0;
      digitalWrite ( 2, LOW); //диод 2 откл
      digitalWrite ( 6, LOW);
    }
    shift_bt_prev = 1;
    delay(20);
  }
  if (encoder_A == 0 && shift_bt_prev == 1) {
    shift_bt_prev = 0;
  }

  //задание банок энкодеров
  encoder_A = digitalRead(11);
  if (encoder_A == 1 && bt_prev [13] == 0) {
    if (en_bank != 0) {
      en_bank = 0;
      digitalWrite ( 5, LOW);
      digitalWrite ( 4, LOW); //диод 3 откл
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
      digitalWrite ( 4, LOW); //диод 3 откл
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
      digitalWrite ( 4, HIGH); //диод 3 откл
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
      digitalWrite ( 4, HIGH); //диод 3 откл
    }
    bt_prev [16] = 1;
    delay(20);
  }
  if (encoder_A == 0 &&  bt_prev [16] == 1) {
    bt_prev [16] = 0;
  }

  // обработка кнопок
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
  //обработка энкодеров
  for (i = 0; i <= 9; i++) {
    encoder_A = digitalRead(en_A[i]);     // считываем состояние выхода А энкодера
    if ( encoder_A == 0 && en_prev[i] == 1 ) {  // если состояние изменилось с положительного к нулю
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
