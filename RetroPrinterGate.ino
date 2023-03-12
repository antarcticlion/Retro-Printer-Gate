/*****************************************************************

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

*****************************************************************/

#include <SoftwareSerial.h>

/*****************************************************************/
// gpio pinout description

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


/*****************************************************************/
// レジスタ設定

#define PINB *((volatile uint8_t *)(0x23))   // Port B トグル
#define DDRB *((volatile uint8_t *)(0x24))   // Port B ディレクション
#define PORTB *((volatile uint8_t *)(0x25))  // Port B
#define PINC *((volatile uint8_t *)(0x26))   // Port C トグル
#define DDRC *((volatile uint8_t *)(0x27))   // Port C ディレクション
#define PORTC *((volatile uint8_t *)(0x28))  // Port C
#define PIND *((volatile uint8_t *)(0x29))   // Port D トグル
#define DDRD *((volatile uint8_t *)(0x2A))   // Port D ディレクション
#define PORTD *((volatile uint8_t *)(0x2B))  // Port D

/*****************************************************************/
// Misc

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

const uint8_t mode_select[] = {
  MODE_UART_TO_CENTRO,
  MODE_USB_TO_CENTRO,
  MODE_CENTRO_TO_UART,
  MODE_CENTRO_TO_USB,
  MODE_UART_TO_CENTRO,
  MODE_USB_TO_CENTRO,
  MODE_CENTRO_TO_UART,
  MODE_CENTRO_TO_USB,
  MODE_UART_TO_CENTRO,
  MODE_USB_TO_CENTRO,
  MODE_CENTRO_TO_UART,
  MODE_CENTRO_TO_USB,
  MODE_UART_TO_CENTRO,
  MODE_USB_TO_CENTRO,
  MODE_CENTRO_TO_UART,
  MODE_CENTRO_TO_USB,
};

const bool usb_select[] = {
  false,
  true,
  false,
  true,
  false,
  true,
  false,
  true,
  false,
  true,
  false,
  true,
  false,
  true,
  false,
  true,
};

const uint32_t baudrate_select[] = {
  BAUDRATE_9600,
  BAUDRATE_9600,
  BAUDRATE_9600,
  BAUDRATE_9600,
  BAUDRATE_19200,
  BAUDRATE_19200,
  BAUDRATE_19200,
  BAUDRATE_19200,
  BAUDRATE_57600,
  BAUDRATE_57600,
  BAUDRATE_57600,
  BAUDRATE_57600,
  BAUDRATE_115200,
  BAUDRATE_115200,
  BAUDRATE_115200,
  BAUDRATE_115200,
};

SoftwareSerial UARTport(12, 13);

static uint8_t curr_state = MODE_UART_TO_CENTRO;
static uint8_t curr_flow = XON;

/*****************************************************************/
// LED制御

static uint8_t led1_dulation = 0;
static uint8_t led2_dulation = 0;
static uint8_t led1_idle = 0;
static uint8_t led2_idle = 0;


/*****************************************************************/
// リングバッファ関係

//#define PRINT_BUFF_SIZE 8        // Debug
// #define PRINT_BUFF_SIZE 256      // SRAM 1KB
#define PRINT_BUFF_SIZE 1536        // SRAM 2KB
// #define PRINT_BUFF_SIZE 7168     // SRAM 8KB
// #define PRINT_BUFF_SIZE 92160    // SRAM 96KB
// #define PRINT_BUFF_SIZE 256000   // SRAM 262KB
// #define PRINT_BUFF_SIZE 512000   // SRAM 520KB

static uint8_t databuf[PRINT_BUFF_SIZE];
static uint32_t buf_read_index = 0;
static uint32_t buf_write_index = 0;

/*****************************************************************/
// ポート初期化

inline uint8_t init_U2C() {
  DDRB = 0xFE;  // U2C
  DDRD = 0x17;  // U2C
  PORTD = ((PORTD & 0x03) | 0x00);
  PORTB = ((PORTB & 0xFC) | 0x04);
  return 0;
}

inline uint8_t init_C2U() {
  DDRB = 0x02;  // C2U
  DDRD = 0x18;  // C2U
  PORTD = 0xFF; // all pullup
  PORTB &= 0xF7;
  return 0;
}

/*****************************************************************/
// リングバッファ関係

inline uint8_t buf_push(uint8_t cur_data){
  uint32_t tmp_index = buf_write_index;
  ++tmp_index;
  tmp_index %= PRINT_BUFF_SIZE;
  if(buf_read_index == tmp_index){
    return -1;
  }
  buf_write_index = tmp_index;
  databuf[buf_write_index] = cur_data;
//  Serial.print("Buf push Char : ");
//  Serial.println(cur_data);
  return 0;
}

inline uint8_t buf_pop(uint8_t *cur_data){
  if(buf_read_index == buf_write_index){
    return -1;
  }
  ++buf_read_index;
  buf_read_index %= PRINT_BUFF_SIZE;
  *cur_data = databuf[buf_read_index];
//  Serial.print("Buf pop Char : ");
//  Serial.println(*cur_data);
}

inline uint32_t buf_available(){
  uint32_t result = PRINT_BUFF_SIZE;
  result += buf_write_index;
  result -= buf_read_index;
  result %= PRINT_BUFF_SIZE;
  return result;
}

inline uint8_t buf_isfull(){
  return ((buf_available() < (PRINT_BUFF_SIZE - 1)) ? 0 : -1);
}

/*****************************************************************/
// 初期化

void setup() {
  { //LED off
    DDRC = 0x30;
    PORTC = 0x0F;
  }

  { //Ring buffer initialize and flow control flag initialize
    for (uint32_t i = 0; i < PRINT_BUFF_SIZE; i++) {
      databuf[i] = 0;
    }
    buf_read_index = 0;
    buf_write_index = 0;
    curr_flow = XON;
  }

  { // mode selection and port init
    uint8_t dipsw = PINC & 0x0F;
    bool usb = usb_select[dipsw];
    curr_state = mode_select[dipsw];
    
    if(curr_state == MODE_UART_TO_CENTRO) init_U2C();
    if(curr_state == MODE_USB_TO_CENTRO)  init_U2C();
    if(curr_state == MODE_CENTRO_TO_UART) init_C2U();
    if(curr_state == MODE_CENTRO_TO_USB)  init_C2U();
    if(usb){
      Serial.begin(baudrate_select[dipsw]);
      Serial.write(XON);
    }else{
      Serial.begin(9600);
      UARTport.begin(baudrate_select[dipsw]);
      UARTport.write(XON);
    }

    if(curr_state == MODE_UART_TO_CENTRO){
      PORTC |= 0x10;
      delay(250);
      PINC = 0x30;
      delay(250);
      PINC = 0x30;
      delay(250);
      PINC = 0x30;
      delay(250);
      PORTC |= 0x30;
      delay(250);
      PINC = 0x30;
    }
    if(curr_state == MODE_USB_TO_CENTRO){
      PORTC |= 0x10;
      delay(125);
      PINC = 0x30;
      delay(125);
      PINC = 0x30;
      delay(125);
      PINC = 0x30;
      delay(125);
      PORTC |= 0x30;
      delay(125);
      PINC = 0x30;
    }
    if(curr_state == MODE_CENTRO_TO_UART){
      PORTC |= 0x30;
      delay(250);
      PINC = 0x30;
      delay(250);
      PINC = 0x30;
      delay(250);
      PINC = 0x30;
      delay(250);
      PORTC |= 0x30;
      delay(250);
      PINC = 0x30;
    }
    if(curr_state == MODE_CENTRO_TO_USB){
      PORTC |= 0x30;
      delay(125);
      PINC = 0x30;
      delay(125);
      PINC = 0x30;
      delay(125);
      PINC = 0x30;
      delay(125);
      PORTC |= 0x30;
      delay(125);
      PINC = 0x30;
    }
  }
}

/*****************************************************************/
// データ転送　UART　→　セントロ

inline void octet_handover_uart_to_centro(bool usb){
  uint8_t octet = 0;
  if(!usb)UARTport.listen();
  while(usb ? Serial.available() : UARTport.available()) {
    if(buf_isfull()){
      if(curr_flow == XON){
        (usb ? Serial.write(XOFF) : UARTport.write(XOFF));
        curr_flow = XOFF;
      }
      break;
    }
    buf_push(usb ? Serial.read() : UARTport.read());
    if(buf_isfull()){
      if(curr_flow == XON){
        UARTport.write(XOFF);
        curr_flow = XOFF;
      }
      break;
    }
  }
  if(buf_available()){
    buf_pop(&octet);
    PORTD = ((PORTD & 0x03) | (octet << 2));
    PORTB = ((PORTB & 0xFC) | (octet >> 6));
    PORTB |= 0x04;
    if(curr_flow == XON){
      (usb ? Serial.write(XOFF) : UARTport.write(XOFF));
      curr_flow = XOFF;
    }
    delayMicroseconds(1);
    PORTB &= 0xFB;
    while (PORTB & 0x08);
    (usb ? Serial.write(XON) : UARTport.write(XON));
    curr_flow = XON;
    led1_dulation = 500;
    PORTC |= 0x10;
    delayMicroseconds(2);
  }
}

/*****************************************************************/
// バッファから1オクテットUARTに転送（フロー制御付き）

void send_octet_buf_to_uart(bool usb)  {
  uint8_t octet;
  if(!usb)UARTport.listen();
  if(buf_available()){
    if(usb ? Serial.available() : UARTport.available()){
      octet = (usb ? Serial.read() : UARTport.read());
      switch(octet){
      case XON:
        if(curr_flow == XOFF){
          (usb ? Serial.write(XON) : UARTport.write(XON));
          curr_flow = XON;
        }
        break;
      case XOFF:
        if(curr_flow == XON){
          (usb ? Serial.write(XOFF) : UARTport.write(XOFF));
          curr_flow = XOFF;
        }
        break;
      };
    }
    if(curr_flow == XON){
      buf_pop(&octet);
      (usb ? Serial.write(octet) : UARTport.write(octet));
    }
  }
}

/*****************************************************************/
// データ転送　セントロ　→　UART

inline void octet_handover_centro_to_uart(bool usb){
  uint8_t octet = 0;
  if (PORTB & 0x04) {     // strobe == H?
    PORTB |= 0x08;        // busy = H
    while((!buf_isfull()) && (PORTB & 0x04)){  // buf not full && strobe == L?
      send_octet_buf_to_uart(usb);
    }
    octet = PORTB;
    octet <<= 6;
    octet |= (PORTD >> 2);
    buf_push(octet);
    while(buf_isfull()){
      send_octet_buf_to_uart(usb);
    } 
    PORTB &= 0xF7;
    led2_dulation = 500;
    PORTC |= 0x20;
  }
}

/*****************************************************************/
// メインループ
void loop() {
  uint8_t octet = 0;
  switch (curr_state) {
  /*--------------------------------------*/
  /* UART TTLからセントロニクス */
  case MODE_UART_TO_CENTRO:
    octet_handover_uart_to_centro(false);
    break;
  /*--------------------------------------*/
  /* USB Serialからセントロニクス */
  case MODE_USB_TO_CENTRO:
    octet_handover_uart_to_centro(true);
    break;
  /*--------------------------------------*/
  /* セントロニクスからUART TTL */
  case MODE_CENTRO_TO_UART:
    octet_handover_centro_to_uart(false);
    break;
  /*--------------------------------------*/
  /* セントロニクスからUSB Serial */
  case MODE_CENTRO_TO_USB:
    octet_handover_centro_to_uart(true);
    break;
  /*--------------------------------------*/
  /* 異常処理 */
  default:
    PORTC = 0x10;
    while (1) {
      delay(250);
      PINC = 0x30;
    }
  }
  /*--------------------------------------*/
  /* LED制御 */
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
