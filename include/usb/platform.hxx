// SPDX-License-Identifier: BSD-3-Clause
#ifndef USB_PLATFORM_HXX
#define USB_PLATFORM_HXX

#if defined(TM4C123GH6PM)
#include <tm4c123gh6pm/platform.hxx>
#include <tm4c123gh6pm/constants.hxx>
#elif defined(STM32F1)
#include <stm32f1/platform.hxx>
#include <stm32f1/constants.hxx>
#elif defined(ATXMEGA256A3U)
#include <avr/io.h>
#include "usb/platforms/atxmega256a3u/constants.hxx"
#define USB_MEM_SEGMENTED
#endif

#endif /*USB_PLATFORM_HXX*/
