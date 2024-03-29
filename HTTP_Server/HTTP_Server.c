/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include <stdio.h>

#include "main.h"

#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE

//#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
//#include "Board_LED.h"                  // ::Board Support:LED
//#include "Board_Buttons.h"              // ::Board Support:Buttons
//#include "Board_ADC.h"                  // ::Board Support:A/D Converter
//#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
//#include "GLCD_Config.h"                // Keil.MCBSTM32F400::Board Support:Graphic LCD
#include "lcd.h"

// Main stack size must be multiple of 8 Bytes
#define APP_MAIN_STK_SZ (1024U)
uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};

//extern GLCD_FONT GLCD_Font_6x8;
//extern GLCD_FONT GLCD_Font_16x24;

//extern uint16_t AD_in          (uint32_t ch);
//extern uint8_t  get_button     (void);
//extern void     netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len);

//extern bool LEDrun;
//extern char lcd_text[2][20+1];

//extern osThreadId_t TID_Display;
//extern osThreadId_t TID_Led;

//bool LEDrun;
//char lcd_text[2][20+1] = { "LCD line 1",
//                           "LCD line 2" };

extern char textL1[20];
extern char textL2[20];
extern uint8_t posicion_L1;
extern uint8_t posicion_L2;

///* Thread IDs */
//extern osThreadId_t tid_ThDisplay; // thread id
osThreadId_t tid_ThDisplay; // thread id
//osThreadId_t TID_Led;

///* Thread declarations */
//static void BlinkLed (void *arg);
//static __NO_RETURN void Thread_Display (void *arg); // thread function
void Thread_Display (void *arg); // thread function
int Init_Thread_Display (void);

__NO_RETURN void app_main (void *arg); // thread function principal

static void Led_Init(void);

static void Led_Init(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL; //No haría falta ponerlo porque los niveles me los pone la configuración PP
	//GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; //No hace falta además le llegan 21MHz(42MHz/2)
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

///* Read analog inputs */
//uint16_t AD_in (uint32_t ch) {
//  int32_t val = 0;

//  if (ch == 0) {
//    ADC_StartConversion();
//    while (ADC_ConversionDone () < 0);
//    val = ADC_GetValue();
//  }
//  return ((uint16_t)val);
//}

///* Read digital inputs */
//uint8_t get_button (void) {
//  return ((uint8_t)Buttons_GetState ());
//}

/* IP address change notification */
//void netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len) {

//  (void)if_num;
//  (void)val;
//  (void)len;

//  if (option == NET_DHCP_OPTION_IP_ADDRESS) {
//    /* IP address change, trigger LCD update */
//    osThreadFlagsSet (TID_Display, 0x01);
//  }
//}

///*----------------------------------------------------------------------------
//  Thread 'Display': LCD display handler
// *---------------------------------------------------------------------------*/
//static __NO_RETURN void Display (void *arg) {
//  static uint8_t ip_addr[NET_ADDR_IP6_LEN];
//  static char    ip_ascii[40];
//  static char    buf[24];
//  uint32_t x = 0;

//  (void)arg;

//  GLCD_Initialize         ();
//  GLCD_SetBackgroundColor (GLCD_COLOR_BLUE);
//  GLCD_SetForegroundColor (GLCD_COLOR_WHITE);
//  GLCD_ClearScreen        ();
//  GLCD_SetFont            (&GLCD_Font_16x24);
//  GLCD_DrawString         (x*16U, 1U*24U, "       MDK-MW       ");
//  GLCD_DrawString         (x*16U, 2U*24U, "HTTP Server example ");

//  GLCD_DrawString (x*16U, 4U*24U, "IP4:Waiting for DHCP");

//  /* Print Link-local IPv6 address */
//  netIF_GetOption (NET_IF_CLASS_ETH,
//                   netIF_OptionIP6_LinkLocalAddress, ip_addr, sizeof(ip_addr));

//  netIP_ntoa(NET_ADDR_IP6, ip_addr, ip_ascii, sizeof(ip_ascii));

//  sprintf (buf, "IP6:%.16s", ip_ascii);
//  GLCD_DrawString ( x    *16U, 5U*24U, buf);
//  sprintf (buf, "%s", ip_ascii+16);
//  GLCD_DrawString ((x+10U)*16U, 6U*24U, buf);

//  while(1) {
//    /* Wait for signal from DHCP */
//    osThreadFlagsWait (0x01U, osFlagsWaitAny, osWaitForever);

//    /* Retrieve and print IPv4 address */
//    netIF_GetOption (NET_IF_CLASS_ETH,
//                     netIF_OptionIP4_Address, ip_addr, sizeof(ip_addr));

//    netIP_ntoa (NET_ADDR_IP4, ip_addr, ip_ascii, sizeof(ip_ascii));

//    sprintf (buf, "IP4:%-16s",ip_ascii);
//    GLCD_DrawString (x*16U, 4U*24U, buf);

//    /* Display user text lines */
//    sprintf (buf, "%-20s", lcd_text[0]);
//    GLCD_DrawString (x*16U, 7U*24U, buf);
//    sprintf (buf, "%-20s", lcd_text[1]);
//    GLCD_DrawString (x*16U, 8U*24U, buf);
//  }
//}

///*----------------------------------------------------------------------------
//  Thread 'BlinkLed': Blink the LEDs on an eval board
// *---------------------------------------------------------------------------*/
//static __NO_RETURN void BlinkLed (void *arg) {
//  const uint8_t led_val[16] = { 0x48,0x88,0x84,0x44,0x42,0x22,0x21,0x11,
//                                0x12,0x0A,0x0C,0x14,0x18,0x28,0x30,0x50 };
//  uint32_t cnt = 0U;

//  (void)arg;

//  LEDrun = true;
//  while(1) {
//    /* Every 100 ms */
//    if (LEDrun == true) {
//      LED_SetOut (led_val[cnt]);
//      if (++cnt >= sizeof(led_val)) {
//        cnt = 0U;
//      }
//    }
//    osDelay (100);
//  }
//}

///*----------------------------------------------------------------------------
//  Thread 'Display': LCD display handler for STM32F429ZiTx
// *---------------------------------------------------------------------------*/
int Init_Thread_Display (void) {
 
  tid_ThDisplay = osThreadNew(Thread_Display, NULL, NULL); // Creacion del hilo del display
  if (tid_ThDisplay == NULL) {
    return(-1);
  }
 
  return(0);
}

//static __NO_RETURN void Thread_Display (void *arg) {
void Thread_Display (void *arg) {
//  static uint8_t ip_addr[NET_ADDR_IP6_LEN];
//  static char    ip_ascii[40];
//  static char    buf[24];
//  uint32_t x = 0;

//  (void)arg;

//  GLCD_Initialize         ();
//  GLCD_SetBackgroundColor (GLCD_COLOR_BLUE);
//  GLCD_SetForegroundColor (GLCD_COLOR_WHITE);
//  GLCD_ClearScreen        ();
//  GLCD_SetFont            (&GLCD_Font_16x24);
//  GLCD_DrawString         (x*16U, 1U*24U, "       MDK-MW       ");
//  GLCD_DrawString         (x*16U, 2U*24U, "HTTP Server example ");

//  GLCD_DrawString (x*16U, 4U*24U, "IP4:Waiting for DHCP");

//  /* Print Link-local IPv6 address */
//  netIF_GetOption (NET_IF_CLASS_ETH,
//                   netIF_OptionIP6_LinkLocalAddress, ip_addr, sizeof(ip_addr));

//  netIP_ntoa(NET_ADDR_IP6, ip_addr, ip_ascii, sizeof(ip_ascii));

//  sprintf (buf, "IP6:%.16s", ip_ascii);
//  GLCD_DrawString ( x    *16U, 5U*24U, buf);
//  sprintf (buf, "%s", ip_ascii+16);
//  GLCD_DrawString ((x+10U)*16U, 6U*24U, buf);

//  while(1) {
//    /* Wait for signal from DHCP */
//    osThreadFlagsWait (0x01U, osFlagsWaitAny, osWaitForever);

//    /* Retrieve and print IPv4 address */
//    netIF_GetOption (NET_IF_CLASS_ETH,
//                     netIF_OptionIP4_Address, ip_addr, sizeof(ip_addr));

//    netIP_ntoa (NET_ADDR_IP4, ip_addr, ip_ascii, sizeof(ip_ascii));

//    sprintf (buf, "IP4:%-16s",ip_ascii);
//    GLCD_DrawString (x*16U, 4U*24U, buf);

//    /* Display user text lines */
//    sprintf (buf, "%-20s", lcd_text[0]);
//    GLCD_DrawString (x*16U, 7U*24U, buf);
//    sprintf (buf, "%-20s", lcd_text[1]);
//    GLCD_DrawString (x*16U, 8U*24U, buf);
//  }
	//uint32_t flags;

  while(1) {
		/* Wait for signal from DHCP */
    osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
		
		/* Display user text lines */
		posicion_L1 = 0;
		posicion_L2 = 0;
		textToLocalBufferL1(textL1);
		textToLocalBufferL2(textL2);
		LCD_update();
		
		//osThreadYield();                            // suspend thread
	}	
}


/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
__NO_RETURN void app_main (void *arg) {
  (void)arg;

//  LED_Initialize();
//  Buttons_Initialize();
//  ADC_Initialize();
	/*  Inicializar los LEDs */
	Led_Init(); 
	/* Inicializar Display */
	LCD_Clean();
	LCD_Reset();
	LCD_Init();

	
  netInitialize(); // Inicializar Network Component y la interfaz

//  TID_Led     = osThreadNew (BlinkLed, NULL, NULL);
//  Th_Display = osThreadNew (Thread_Display,  NULL, NULL);
	
	//Init_Thread_Display(); // Inicializar el thread para el display
	
	tid_ThDisplay = osThreadNew(Thread_Display, NULL, NULL);

  osThreadExit(); // Termina la ejecución de este hilo que estaba corriendo
}
