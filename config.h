/* *****************************************************************************
 * The MIT License
 *
 * Copyright (c) 2010 LeafLabs LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * ****************************************************************************/

/**
 *  @file config.h
 *
 *  @brief bootloader settings and macro defines
 *
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef Maple

# define LED_BANK GPIOA
# define LED_RCC_GPIO RCC_GPIOA
# define LED      GPIO5

# define NoButton
# define UCF5k
# define BUTTON_BANK GPIOC
# define BUTTON_RCC_GPIO RCC_GPIOC
# define BUTTON      GPIO9

# define USB_DISC_BANK GPIOC
# define USB_DISC_RCC_GPIO RCC_GPIOC
# define USB_DISC      GPIO12

#elif defined MapleMini

# define LED_BANK GPIOB
# define LED_RCC_GPIO RCC_GPIOB
# define LED      GPIO1

# define NoButton
# define UCF5k
# define BUTTON_BANK GPIOB
# define BUTTON_RCC_GPIO RCC_GPIOB
# define BUTTON      GPIO8

# define USB_DISC_BANK GPIOB
# define USB_DISC_RCC_GPIO RCC_GPIOB
# define USB_DISC      GPIO9

#elif defined MapleMini_2k

# define LED_BANK GPIOB
# define LED_RCC_GPIO RCC_GPIOB
# define LED      GPIO1

# define NoButton
# define BUTTON_BANK GPIOB
# define BUTTON_RCC_GPIO RCC_GPIOB
# define BUTTON      GPIO8

# define USB_DISC_BANK GPIOB
# define USB_DISC_RCC_GPIO RCC_GPIOB
# define USB_DISC      GPIO9

#elif defined Blue

# define LED_BANK GPIOA
# define LED_RCC_GPIO RCC_GPIOA
# define LED      GPIO9
# define NO_USB_DISC
# define NoButton

#elif defined Blue_noHW

# define LED_BANK GPIOA
# define LED_RCC_GPIO RCC_GPIOA
# define LED      GPIO9
# define NO_USB_DISC_HW
# define NoButton

#elif defined Dev

# define LED_BANK GPIOB
# define LED_RCC_GPIO RCC_GPIOB
# define LED      GPIO13
# define NO_USB_DISC_HW
# define NoButton

#elif defined Red

# define LED_BANK GPIOA
# define LED_RCC_GPIO RCC_GPIOA
# define LED      GPIO9
# define NO_USB_DISC
# define NoButton

#elif defined Red_noHW

# define LED_BANK GPIOA
# define LED_RCC_GPIO RCC_GPIOA
# define LED      GPIO9
# define NO_USB_DISC_HW
# define NoButton

#elif defined blueDev

# define LED_BANK GPIOC
# define LED_RCC_GPIO RCC_GPIOC
# define LED      GPIO13
# define NO_USB_DISC_HW
# define NoButton

#elif defined blackDev

# define LED_BANK GPIOB
# define LED_RCC_GPIO RCC_GPIOB
# define LED      GPIO12
# define NO_USB_DISC_HW
# define NoButton

#else
# error "Unknown platform"
#endif

#ifndef UCF5k
#define USER_CODE_FLASH   ((uint32_t)0x08002000)
#else
#define USER_CODE_FLASH   ((uint32_t)0x08005000)
#endif
#define FLASH_END         ((uint32_t)0x08020000)

#endif
