/*
Retro Printer Gate

Copyright (c) 2023 antarcticlion

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.
*/

//PB5 D13 Software serial RXD
//PB4 D12 Software serial TXD
//PB3 D11 BUSY (Active High)
//PB2 D10 STROBE (Active Low, 1us or longer pulse, Rise edge trigger)
//PB1 D9 PA7
//PB0 D8 PA6

//PC5 A5 LED2
//PC4 A4 LED1
//PC3 A3 DIP SW3
//PC2 A2 DIP SW2
//PC1 A1 DIP SW1
//PC0 A0 DIP SW0

//PD7 D7 PA7
//PD7 D6 PA6
//PD7 D5 PA5
//PD7 D4 PA4
//PD7 D3 PA3
//PD7 D2 PA2
//PD7 D1 PA1
//PD7 D0 PA0

#include <SoftwareSerial.h>
#define PINB *((volatile uint8_t *)(0x23))   // Port B トグル
#define DDRB *((volatile uint8_t *)(0x24))   // Port B ディレクション
#define PORTB *((volatile uint8_t *)(0x25))  // Port B
#define PINC *((volatile uint8_t *)(0x26))   // Port C トグル
#define DDRC *((volatile uint8_t *)(0x27))   // Port C ディレクション
#define PORTC *((volatile uint8_t *)(0x28))  // Port C
#define PIND *((volatile uint8_t *)(0x29))   // Port D トグル
#define DDRD *((volatile uint8_t *)(0x2A))   // Port D ディレクション
#define PORTD *((volatile uint8_t *)(0x2B))  // Port D

#define XON (0x11)
#define XOFF (0x13)

#define BAUDRATE_9600 9600
#define BAUDRATE_19200 19200
#define BAUDRATE_57600 57600
#define BAUDRATE_115200 115200

enum modes {
  MODE_UART_TO_CENTRO = 0,
  MODE_USB_TO_CENTRO,
  MODE_CENTRO_TO_UART,
  MODE_CENTRO_TO_USB,
};

SoftwareSerial UARTport(12, 13);

static uint8_t led1_dulation = 0;
static int8_t led2_dulation = 0;
static uint8_t led1_idle = 0;
static uint8_t led2_idle = 0;

static uint8_t curr_state = MODE_UART_TO_CENTRO;

inline void init_U2C(){
  DDRB = 0xFE;  // U2C
  DDRD = 0x17;  // U2C
  PORTD = ((PORTD & 0x03) | 0x00);
  PORTB = ((PORTB & 0xFC) | 0x04);
}
inline void init_C2U(){
  DDRB = 0x02;  // C2U
  DDRD = 0x18;  // C2U
  PORTB &= 0xF7;
}

void setup() {
  DDRC = 0x30;
  PORTC = 0x00;

  switch (PINC & 0x0F) {
    case 0x00:
      //UART TO CENTRO  9600baud
      curr_state = MODE_UART_TO_CENTRO;
      init_U2C();
      UARTport.begin(BAUDRATE_9600);
      UARTport.write(XON);
      break;

    case 0x01:
      //USB TO CENTRO  9600baud
      curr_state = MODE_USB_TO_CENTRO;
      init_U2C();
      Serial.begin(BAUDRATE_9600);
      Serial.write(XON);
      break;

    case 0x02:
      //CENTRO TO UART  9600baud
      curr_state = MODE_CENTRO_TO_UART;
      init_C2U();
      UARTport.begin(BAUDRATE_9600);
      UARTport.write(XON);
      break;

    case 0x03:
      //CENTRO TO USB  9600baud
      curr_state = MODE_CENTRO_TO_USB;
      init_C2U();
      Serial.begin(BAUDRATE_9600);
      Serial.write(XON);
      break;

    case 0x04:
      //UART TO CENTRO  19200baud
      curr_state = MODE_UART_TO_CENTRO;
      init_U2C();
      UARTport.begin(BAUDRATE_19200);
      UARTport.write(XON);
      break;

    case 0x05:
      //USB TO CENTRO  19200baud
      curr_state = MODE_USB_TO_CENTRO;
      init_U2C();
      Serial.begin(BAUDRATE_19200);
      Serial.write(XON);
      break;

    case 0x06:
      //CENTRO TO UART  19200baud
      curr_state = MODE_CENTRO_TO_UART;
      init_C2U();
      UARTport.begin(BAUDRATE_19200);
      UARTport.write(XON);
      break;

    case 0x07:
      //CENTRO TO USB  19200baud
      curr_state = MODE_CENTRO_TO_USB;
      init_C2U();
      Serial.begin(BAUDRATE_19200);
      Serial.write(XON);
      break;

    case 0x08:
      //UART TO CENTRO  57600baud
      curr_state = MODE_UART_TO_CENTRO;
      init_U2C();
      UARTport.begin(BAUDRATE_57600);
      UARTport.write(XON);
      break;

    case 0x09:
      //USB TO CENTRO  57600baud
      curr_state = MODE_USB_TO_CENTRO;
      init_U2C();
      Serial.begin(BAUDRATE_57600);
      Serial.write(XON);
      break;

    case 0x0A:
      //CENTRO TO UART  57600baud
      curr_state = MODE_CENTRO_TO_UART;
      init_C2U();
      UARTport.begin(BAUDRATE_57600);
      UARTport.write(XON);
      break;

    case 0x0B:
      //CENTRO TO USB  57600baud
      curr_state = MODE_CENTRO_TO_USB;
      init_C2U();
      Serial.begin(BAUDRATE_57600);
      Serial.write(XON);
      break;

    case 0x0C:
      //UART TO CENTRO  115200baud
      curr_state = MODE_UART_TO_CENTRO;
      init_U2C();
      UARTport.begin(BAUDRATE_115200);
      UARTport.write(XON);
      break;

    case 0x0D:
      //USB TO CENTRO  115200baud
      curr_state = MODE_USB_TO_CENTRO;
      init_U2C();
      Serial.begin(BAUDRATE_115200);
      Serial.write(XON);
      break;

    case 0x0E:
      //CENTRO TO UART  115200baud
      curr_state = MODE_CENTRO_TO_UART;
      init_C2U();
      UARTport.begin(BAUDRATE_115200);
      UARTport.write(XON);
      break;

    case 0x0F:
      //CENTRO TO USB  115200baud
      curr_state = MODE_CENTRO_TO_USB;
      init_C2U();
      Serial.begin(BAUDRATE_115200);
      Serial.write(XON);
      break;

    default:
      PORTC = 0x10;
      while (1) {
        delay(500);
        PINC = 0x30;
      }
  }
}

void loop() {
  uint8_t octet = 0;
  switch (curr_state) {
    case MODE_UART_TO_CENTRO:
      UARTport.listen();
      UARTport.write(XOFF);
      while (UARTport.available()) {
        octet = UARTport.read();
        PORTD = ((PORTD & 0x03) | (octet << 2));
        PORTB = ((PORTB & 0xFC) | (octet >> 2));
        PORTB |= 0x04;
        delayMicroseconds(1);
        PORTB &= 0xFB;
        while (PORTB & 0x08)
          ;
        led1_dulation = 500;
        PORTC |= 0x10;
      }
      UARTport.write(XON);
      break;
    case MODE_USB_TO_CENTRO:
      Serial.write(XOFF);
      while (Serial.available()) {
        octet = Serial.read();
        PORTD = ((PORTD & 0x03) | (octet << 2));
        PORTB = ((PORTB & 0xFC) | (octet >> 2));
        PORTB |= 0x04;
        delayMicroseconds(1);
        PORTB &= 0xFB;
        while (PORTB & 0x08)
          ;
        led1_dulation = 500;
        PORTC |= 0x10;
      }
      Serial.write(XON);
      break;
    case MODE_CENTRO_TO_UART:
      if (PORTB & 0x04) {
        PORTB |= 0x08;
        while (PORTB & 0x04);
        octet = PORTB;
        octet <<= 6;
        octet |= (PORTD >> 2);
        UARTport.write(octet);
        PORTB &= 0xF7;
        led2_dulation = 500;
        PORTC |= 0x20;
      }
      break;
    case MODE_CENTRO_TO_USB:
      if (PORTB & 0x04) {
        PORTB |= 0x08;
        while (PORTB & 0x04);
        octet = PORTB;
        octet <<= 6;
        octet |= (PORTD >> 2);
        Serial.write(octet);
        PORTB &= 0xF7;
        led2_dulation = 500;
        PORTC |= 0x20;
      }
      break;
    default:
      PORTC = 0x10;
      while (1) {
        delay(250);
        PINC = 0x30;
      }
  }
  if (led1_idle) {
    led1_idle--;
  } else {
    if (led1_dulation) {
      led1_dulation--;
      if (!led1_dulation) {
        PORTC &= 0xE0;
        led1_idle = 250;
      }
    }
  }
  if (led2_idle) {
    led2_idle--;
  } else {
    if (led2_dulation) {
      led2_dulation--;
      if (!led2_dulation) {
        PORTC &= 0xE0;
        led2_idle = 250;
      }
    }
  }
}
