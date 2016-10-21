/*
 * emb6 is licensed under the 3-clause BSD license. This license gives everyone
 * the right to use and distribute the code, either in binary or source code
 * format, as long as the copyright license is retained in the source code.
 *
 * The emb6 is derived from the Contiki OS platform with the explicit approval
 * from Adam Dunkels. However, emb6 is made independent from the OS through the
 * removal of protothreads. In addition, APIs are made more flexible to gain
 * more adaptivity during run-time.
 *
 * The license text is:
 *
 * Copyright (c) 2015,
 * Hochschule Offenburg, University of Applied Sciences
 * Laboratory Embedded Systems and Communications Electronics.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*============================================================================*/
/**
 *      \addtogroup emb6
 *      @{
 *   \addtogroup demo
 *   @{
 *   \addtogroup demo_coap
 *   @{
 *   \defgroup demo_coap_server CoAP server
 *
 *   Simple example of a CoAP Server requesting several resources.
 *   @{
*/
/*! \file   demo_coap_srv.h

    \author Peter Lehmann, peter.lehmann@hs-offenburg.de

    \brief  This is the header file of the demo CoAP server application

    \version 0.0.1
*/
#ifndef DEMO_COAP_SRV_H_
#define DEMO_COAP_SRV_H_

/*==============================================================================
                              INCLUDES
==============================================================================*/
#include "emb6.h"

/*==============================================================================
                                     MACROS
 =============================================================================*/
#define DISPLAY_TEXT_LENGTH     65

/*==============================================================================
                          LOCAL VARIABLE DECLARATIONS
 =============================================================================*/
typedef struct
{
    int en;
    int delay;

}s_led_task_param_t;


typedef struct
{
    char output[DISPLAY_TEXT_LENGTH];
    int len;

}s_display_output_t;

/*==============================================================================
                         FUNCTION PROTOTYPES OF THE API
==============================================================================*/

/*============================================================================*/
/*!
   \brief Initialization of the CoAP server application.

*/
/*============================================================================*/
int8_t demo_coapInit(void);

/*============================================================================*/
/*!
    \brief Configuration of the CoAP server application.

    \return 0 - error, 1 - success
*/
/*============================================================================*/
uint8_t demo_coapConf(s_ns_t* pst_netStack);

#endif /* DEMO_COAP_SRV_H_ */
/** @} */
/** @} */
/** @} */
/** @} */