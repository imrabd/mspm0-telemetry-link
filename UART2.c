/* UART2.c
 * Imran Abdullah
 * Date: April 6, 2026
 * PA22 UART2 Rx from other microcontroller PA8 UART1 Tx<br>
 */

#include <ti/devices/msp/msp.h>
#include "UART2.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/FIFO1.h"
#define PA8INDEX  18 // UART1_TX  SPI0_CS0  UART0_RTS TIMA0_C0  TIMA1_C0N
#define PA9INDEX  19 // UART1_RX  SPI0_PICO UART0_CTS TIMA0_C1  RTC_OUT   TIMA0_C0N TIMA1_C1N CLK_OUT
#define PA22INDEX 46 // UART2_RX  TIMG8_C1  UART1_RTS TIMA0_C1  CLK_OUT   TIMA0_C0N TIMG6_C1

uint32_t LostData;

// power Domain PD0
// for 80MHz bus clock, UART clock is ULPCLK 40MHz
// initialize UART2 for 2000 baud rate
// no transmit, interrupt on receive timeout
void UART2_Init(void){
    // do not reset or activate PortA, already done in LaunchPad_Init
    // RSTCLR to UART2 peripherals
    //   bits 31-24 unlock key 0xB1
    //   bit 1 is Clear reset sticky bit
    //   bit 0 is reset gpio port

    // Enable power to UART1 peripherals
     // write this

    UART2->GPRCM.RSTCTL = 0xB1000003;
    UART2->GPRCM.PWREN  = 0x26000001;
    Clock_Delay(24);
    IOMUX->SECCFG.PINCM[PA22INDEX] = 0x00040082;
    Fifo1_Init();
    UART2->CLKSEL = 0x08;
    UART2->CLKDIV = 0x00;
    UART2->CTL0 &= ~0x01;
    UART2->CTL0 = 0x00020018;
    UART2->IBRD = 1250;
    UART2->FBRD = 0;
    UART2->LCRH = 0x00000030;
    UART2->CPU_INT.IMASK = 0x0001;
    UART2->IFLS = 0x0422;
    NVIC->ICPR[0] = 1 << 14;
    NVIC->ISER[0] = 1 << 14;
    NVIC->IP[3] = (NVIC->IP[3] & (~0x00C00000)) | (2 << 22);
    UART2->CTL0 |= 0x01;
}
//------------UART2_InChar------------
// Get new serial port receive data from FIFO1
// Input: none
// Output: Return 0 if the FIFO1 is empty
//         Return nonzero data from the FIFO1 if available
char UART2_InChar(void){
    return Fifo1_Get();
}


void UART2_IRQHandler(void){ 
// write this
// acknowledge RTOUT
  uint32_t status = UART2->CPU_INT.IIDX;
  if(status == 0x01){
      GPIOB->DOUTTGL31_0 = BLUE;
      GPIOB->DOUTTGL31_0 = BLUE;
      while((UART2->STAT & 0x04) == 0){
          char letter = UART2->RXDATA;
          if(Fifo1_Put(letter) == 0){
              LostData++;
          }
      }
      GPIOB->DOUTTGL31_0 = BLUE;
  }
}
