/*!
 * @code
 *  ___ _____ _   ___ _  _____ ___  ___  ___ ___
 * / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 * \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 * |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 * embedded.connectivity.solutions.==============
 * @endcode
 *
 * @file       cc13xx.c
 * @copyright  STACKFORCE GmbH, Heitersheim, Germany, http://www.stackforce.de
 * @author     STACKFORCE
 * @brief      This is the 6lowpan-stack driver for the cc13xx mcu.
 */

/*! @defgroup emb6_mcu emb6 stack mcu driver
    This group is the mcu driver for the emb6 stack.
  @{  */

/*============================================================================*/
/*                                INCLUDES                                    */
/*============================================================================*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "emb6.h"
#include "hwinit.h"
#include "target.h"
#include "bsp.h"
#include "board_conf.h"

#include "sf_mcu.h"
#include "sf_mcu_timer.h"
#include "sf_uart.h"
#include "bsp_led.h"
#if CC13XX_LCD_ENABLE
#include "lcd_dogm128_6.h"
#endif /* #if CC13XX_LCD_ENABLE */
#include "driverlib/interrupt.h"
#include "rt_tmr.h"

/*! Enable or disable logging. */
#define     LOGGER_ENABLE        LOGGER_HAL
#include    "logger.h"

#ifndef __TI_ARM__
#warning stdout is not redirected!
#endif /* __TI_ARM__ */

/*============================================================================*/
/*                               MACROS                                       */
/*============================================================================*/
 /*! Systicks per second. */
#define TARGET_CFG_SYSTICK_RESOLUTION           (clock_time_t)( 1000u )
 /*! Timer scaler to get systicks */
#define TARGET_CFG_SYSTICK_SCALER               (clock_time_t)(    2u )

/*! Defines the mcu ticks per second. */
#define MCU_TICKS_PER_SECOND                    2000U
/*! Status for succeeded init functions. */
#define MCU_INIT_STATUS_OK                      0x01U
/*! Compares X with @ref MCU_INIT_STATUS_OK. */
#define MCU_INIT_RET_STATUS_CHECK(X)            ( X == MCU_INIT_STATUS_OK )

/*============================================================================*/
/*                                ENUMS                                       */
/*============================================================================*/

/*============================================================================*/
/*                  STRUCTURES AND OTHER TYPEDEFS                             */
/*============================================================================*/

/*============================================================================*/
/*                       LOCAL FUNCTION PROTOTYPES                            */
/*============================================================================*/
static bool _hal_uart_init();
static bool _hal_systick(void);
static void _hal_isrSysTick(uint32_t l_count);
/*============================================================================*/
/*                           LOCAL VARIABLES                                  */
/*============================================================================*/

/*! Hal tick counter */
static clock_time_t volatile hal_ticks;

/*============================================================================*/
/*                           LOCAL FUNCTIONS                                  */
/*============================================================================*/

#ifdef __TI_ARM__
/* The functions fputc and fputs are used to redirect stdout to
 * the UART interface.
 *
 * This functinality can only be used with the __TI_ARM__ compiler! */

/* fputc writes a single character to the UART interface. */
int fputc(int _c, register FILE *_fp)
{
  sf_uart_write((uint8_t*)&_c, 0x01U);
  return (unsigned char)_c;
}

/* fputs writes a string with a defined length to the interface. */
int fputs(const char *_ptr, register FILE *_fp)
{
  uint16_t i_len = strlen(_ptr);
  sf_uart_write((uint8_t*)_ptr, i_len);

  return i_len;
}
#endif /* __TI_ARM__ */

/*!
 * @brief This function initializes the UART interface.
 *
 * @return true for success, false if initialization failed.
 */
bool _hal_uart_init()
{
  bool b_return = false;
  uint8_t p_data[] = { "\r\n\r\n========== BEGIN ===========\r\n\r\n" };
  uint16_t i_dataLen = sizeof(p_data);

  /* Initialize UART */
  b_return = sf_uart_init();

  if (b_return) {
    if (!(sf_uart_write(p_data, i_dataLen) == i_dataLen))
    {
      b_return = false;
    }
  }

  return b_return;
}

/*!
 * @brief This function updates the systicks.
 *
 * It is used as a callback function for the timer module.
 *
 * @param l_count Unused parameter (API compatibility to sf_mcu_timer).
 */
static void _hal_isrSysTick(uint32_t l_count)
{
  /* Increase ticks */
  hal_ticks++;

  /* Check if the timer has to be updated */
  if ((hal_ticks % TARGET_CFG_SYSTICK_SCALER ) == 0)
  {
    rt_tmr_update();
  }
}

/*!
 * @brief This function functions enables to update the systicks.
 *
 * @return true if actions succeeds, else false.
 */
static bool _hal_systick(void)
{
  bool b_success = false;

  /* Configure the timer to call _hal_isrSysTick */
  b_success = sf_mcu_timer_setCallback((fp_mcu_timer_cb) &_hal_isrSysTick);
  if(b_success)
  {
    /* Enable the timer */
    sf_mcu_timer_enable();
  }
  return b_success;
}

/*==============================================================================
                             API FUNCTIONS
 ==============================================================================*/

/*!
 * @brief Disables all interrupts.
 *
 * This function disables all interrupts when the
 * program enters critical sections.
 */
void hal_enterCritical(void)
{
  /* Disable the interrutps */
  sf_mcu_interruptDisable();
} /* hal_enterCritical() */

/*!
 * @brief Enables all interrupts.
 *
 */
void hal_exitCritical(void)
{
  /* Enbale the interrupts */
  sf_mcu_interruptEnable();
}/* hal_exitCritical() */

/*!
 * @brief This function initializes all of the MCU peripherals.
 *
 * @return Status code.
 */
int8_t hal_init(void)
{
  uint8_t c_retStatus = 0U;

#if CC13XX_LCD_ENABLE
  char lcdBuf[LCD_BYTES];
#endif /* #if CC13XX_LCD_ENABLE */

  /* Initialize the mcu */
  c_retStatus = sf_mcu_init();
  if(MCU_INIT_RET_STATUS_CHECK(c_retStatus))
  {
    /* Initialize the timer. MCU_TICKS_PER_SECOND specifies
     * the tick interval. */
    c_retStatus = sf_mcu_timer_init(MCU_TICKS_PER_SECOND);
    if(MCU_INIT_RET_STATUS_CHECK(c_retStatus))
    {
      c_retStatus = _hal_uart_init();
      if(MCU_INIT_RET_STATUS_CHECK(c_retStatus))
      {
        c_retStatus = _hal_systick();
      }
    }
  }

  /* Initialize hal_ticks */
  hal_ticks = 0x00U;

  /* initialize LEDs */
  bspLedInit( BSP_LED_ALL );

#if CC13XX_LCD_ENABLE
  /* initialize LCD */
  lcdSpiInit();
  lcdInit();
  lcdClear();

  /* Send Simple Message */
  lcdBufferClear( lcdBuf );
  lcdSendBuffer( lcdBuf );
#endif /* #if CC13XX_LCD_ENABLE */

  return c_retStatus;
}/* hal_init() */

/*!
 * @brief This function calculates a random number.
 *
 * Function is not used by the stack and thus not implemented.
 *
 * @return Always 1U.
 */
uint8_t hal_getrand(void)
{
  return 1;
}/* hal_getrand() */

/*!
 * @brief This function turns a led off.
 *
 * @param ui_led Descriptor of a led.
 */
void hal_ledOff(uint16_t ui_led)
{
	switch( ui_led )
	{
	    case E_BSP_LED_0:
            bspLedClear( BSP_LED_1 );
            break;

	    case E_BSP_LED_1:
            bspLedClear( BSP_LED_2 );
            break;

	    case E_BSP_LED_2:
            bspLedClear( BSP_LED_3 );
            break;

	    case E_BSP_LED_3:
            bspLedClear( BSP_LED_4 );
            break;
	}
}/* hal_ledOff() */

/*!
 * @brief This function turns a led on.
 *
 *
 * @param ui_led Descriptor of a led.
 */
void hal_ledOn(uint16_t ui_led)
{
	switch( ui_led )
	{
        case E_BSP_LED_0:
            bspLedSet( BSP_LED_1 );
            break;

	    case E_BSP_LED_1:
            bspLedSet( BSP_LED_2 );
            break;

	    case E_BSP_LED_2:
            bspLedSet( BSP_LED_3 );
            break;

	    case E_BSP_LED_3:
            bspLedSet( BSP_LED_4 );
            break;
	}
}/* hal_ledOn() */

/*!
 * @brief This function makes a delay.
 *
 * The delay value should only be a multiple of 500us.
 *
 * @param i_delay Delay in micro seconds.
 */
void hal_delay_us(uint32_t i_delay)
{
  /*
   * Note(s)
   *
   * hal_delay_us() is only called by emb6.c to make a delay multiple of 500us,
   * which is equivalent to 1 systick
   */
  uint32_t tick_stop;

  hal_enterCritical();
  tick_stop = hal_ticks;
  tick_stop += i_delay / 500;
  hal_exitCritical();
  while (tick_stop > hal_ticks)
  {
    /* do nothing */
  }
} /* hal_delay_us() */

/*!
 * @brief This function initializes the given control pin.
 *
 * @param  e_pinType Type of a pin.
 * @return Pointer to a pin. Always NULL, because of integrated transceiver.
 */
void* hal_ctrlPinInit(en_targetExtPin_t e_pinType)
{
  /* Not needed because of integrated IF */
  return NULL;
} /* hal_ctrlPinInit() */

/*!
 * @brief This function sets a particular pin.
 *
 * Not implemented, because of integrated transceiver.
 *
 * @param p_pin Pointer to a pin.
 */
void hal_pinSet(void * p_pin)
{
  /* Not needed because of integrated IF */
} /* hal_pinSet() */

/*!
 * @brief This function clears a particular pin.
 *
 * Not implemented, because of integrated transceiver.
 *
 * @param p_pin Pointer to a pin.
 */
void hal_pinClr(void * p_pin)
{
  /* Not needed because of integrated IF */
} /* hal_pinClr() */

/*!
 * @brief This function returns the pin status.
 *
 * Not implemented, because of integrated transceiver.
 *
 * @param p_pin Pointer to a pin.
 * @return Status of the pin.
 */
uint8_t hal_pinGet(void * p_pin)
{
  /* Not needed because of integrated IF */
  return 0U;
} /* hal_pinGet() */

/*!
 * @brief This function initializes the SPI interface.
 *
 * Not implemented, because of integrated transceiver.
 *
 * @return Pointer to an allocated memory.
 */
void* hal_spiInit(void)
{
  /* Not needed because of integrated IF */
  return NULL;
} /* hal_spiInit() */

/*!
 * @brief This function selects or deselects the SPI slave.
 *
 * Not implemented, because of integrated transceiver.
 *
 * @param p_spi Pointer to a SPI entitiy.
 * @param action true to select, false to deselect an SPI entitiy.
 *
 * @return 1 if action succeeded, else 0.
 */
uint8_t hal_spiSlaveSel(void * p_spi, bool action)
{
  /* Not needed because of integrated IF */
  return 0U;
} /* hal_spiSlaveSel() */

/*!
 * @brief This function reads data from the SPI interface.
 *
 * Not implemented, because of integrated transceiver.
 *
 * @param p_reg Pointer to buffer storing the data.
 * @param i_length Length of data to be received.
 */
uint8_t hal_spiRead(uint8_t * p_reg, uint16_t i_length)
{
  /* Not needed because of integrated IF */
  return 0U;
} /* hal_spiRead() */

/*!
 * @brief This function writes data to the SPI interface.
 *
 * Not implemented, because of integrated transceiver.
 *
 * @param c_value Pointer to the data to write.
 * @param i_length Length of data to be written.
 */
void hal_spiWrite(uint8_t * c_value, uint16_t i_length)
{
  /* Not needed because of integrated IF */
} /* hal_spiWrite() */

/*!
 * @brief This function simultaneously reads and writes data.
 *
 * Not implemented, because of integrated transceiver.
 *
 * @param p_tx Pointer to the data to write.
 * @param p_rx Pointer to the buffer storing the received data.
 * @param leh Size of data to send.
 */
void hal_spiTxRx(uint8_t *p_tx, uint8_t *p_rx, uint16_t len)
{
  /* Not needed because of integrated IF */
}

/*!
 * @brief This function resets the watchdog timer.
 *
 * Function is not used by the stack and thus not implemented.
 */
void hal_watchdogReset(void)
{
  /* Not needed because the stack will not use this function */
} /* hal_watchdogReset() */

/*!
 * @brief This function starts the watchdog timer.
 *
 * Function is not used by the stack and thus not implemented.
 */
void hal_watchdogStart(void)
{
  /* Not needed because the stack will not use this function */
} /* hal_watchdogStart() */

/*!
 * @brief This function stops the watchdog timer.
 *
 * Function is not used by the stack and thus not implemented.
 */
void hal_watchdogStop(void)
{
  /* Not needed because the stack will not use this function */
} /* hal_watchdogStop() */

/*!
 * @brief This function returns the system ticks.
 *
 * @return Current system tick.
 */
clock_time_t hal_getTick(void)
{
  return TmrCurTick;
} /* hal_getTick() */

/*!
 * @brief This function returns seconds.
 *
 * @return Seconds.
 */
clock_time_t hal_getSec(void)
{
  clock_time_t secs = 0;

  /* Calculate the seconds */
  secs = TmrCurTick / TARGET_CFG_SYSTICK_RESOLUTION;

  return secs;
} /* hal_getSec() */

/*!
 * @brief This function returns the time resolution.
 *
 * @return Resolution in ms.
 */
clock_time_t hal_getTRes(void)
{
  return TARGET_CFG_SYSTICK_RESOLUTION ;
} /* hal_getSec() */


void hal_extiRegister(en_targetExtInt_t e_extInt, en_targetIntEdge_t e_edge, pfn_intCallb_t pf_cbFnct)
{
    /* Not used */
} /* hal_extiRegister() */
void hal_extiClear(en_targetExtInt_t e_extInt)
{
    /* Not used */
} /* hal_extiClear() */
void hal_extiEnable(en_targetExtInt_t e_extInt)
{
    /* Not used */
} /* hal_extiEnable() */
void hal_extiDisable(en_targetExtInt_t e_extInt)
{
    /* Not used */
} /* hal_extiDisable() */

/*! @} 6lowpan_mcu */
