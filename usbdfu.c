/*
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2017 Joerg Riechardt
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/dfu.h>
#include <libopencm3/usb/dwc/otg_fs.h>
#include <libopencm3/usb/dwc/otg_common.h>
#include "config.h"
#include <libopencm3/stm32/tools.h>
#include <libopencm3/stm32/st_usbfs.h>

#define APP_ADDRESS	USER_CODE_FLASH
#define MAX_ADDRESS	0x8020000

/* We need a special large control buffer for this device: */
uint8_t usbd_control_buffer[1024];

static enum dfu_state usbdfu_state = STATE_DFU_IDLE;

static struct {
	uint8_t buf[sizeof(usbd_control_buffer)];
	uint16_t len;
	uint32_t addr;
	uint16_t blocknum;
} prog = {.addr = APP_ADDRESS};

const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x1209,
	.idProduct = 0x4443,
	.bcdDevice = 0x0100,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

const struct usb_dfu_descriptor dfu_function = {
	.bLength = sizeof(struct usb_dfu_descriptor),
	.bDescriptorType = DFU_FUNCTIONAL,
	.bmAttributes = USB_DFU_CAN_DOWNLOAD | USB_DFU_CAN_UPLOAD | USB_DFU_WILL_DETACH,
	.wDetachTimeout = 255,
	.wTransferSize = 1024,
	.bcdDFUVersion = 0x0110,
};

const struct usb_interface_descriptor iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 0,
	.bInterfaceClass = 0xFE, /* Device Firmware Upgrade */
	.bInterfaceSubClass = 1,
	.bInterfaceProtocol = 2,
	.iInterface = 3,

	.extra = &dfu_function,
	.extralen = sizeof(dfu_function),
};

const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = &iface,
}};

const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,

	.interface = ifaces,
};

static const char *usb_strings[] = {
	"IRMP STM32 project",
	"STM32 Bootloader",
	"BL 01",
};

static uint8_t usbdfu_getstatus(uint32_t *bwPollTimeout)
{
	switch (usbdfu_state) {
	case STATE_DFU_DNLOAD_SYNC:
		usbdfu_state = STATE_DFU_DNBUSY;
		*bwPollTimeout = 32;
		return DFU_STATUS_OK;
	case STATE_DFU_MANIFEST_SYNC:
		/* Device will reset when read is complete */
		/* skip STATE_DFU_MANIFEST */
		usbdfu_state = STATE_DFU_MANIFEST_WAIT_RESET;
		return DFU_STATUS_OK;
	default:
		return DFU_STATUS_OK;
	}
}

static void usbdfu_getstatus_complete(usbd_device *device,
						struct usb_setup_data *req)
{
	int i;
	(void)req;
	(void)device;

	switch (usbdfu_state) {
	case STATE_DFU_DNBUSY: ;
		uint32_t baseaddr = prog.addr + prog.blocknum * dfu_function.wTransferSize;
		flash_erase_page(baseaddr);
		gpio_set(LED_BANK, LED);
		for (i = 0; i < prog.len; i += 2)
			flash_program_half_word(baseaddr + i, *(uint16_t*)(prog.buf+i));
		gpio_clear(LED_BANK, LED);

		/* We jump straight to dfuDNLOAD-IDLE,
		 * skipping dfuDNLOAD-SYNC
		 */
		usbdfu_state = STATE_DFU_DNLOAD_IDLE;
		return;
	default:
		return;
	}
}

static enum usbd_request_return_codes usbdfu_control_request(usbd_device *device,
				  struct usb_setup_data *req, uint8_t **buf,
				  uint16_t *len,
				  void (**complete)(usbd_device *device,
						struct usb_setup_data *req))
{
	(void)device;

	if ((req->bmRequestType & 0x7F) != 0x21)
		return 0; /* Only accept class request */

	switch (req->bRequest) {
	case DFU_DNLOAD:
		if ((len == NULL) || (*len == 0)) {
			flash_lock();
			usbdfu_state = STATE_DFU_MANIFEST_SYNC;
			return 1;
		} else {
			/* Copy download data for use on GET_STATUS */
			prog.blocknum = req->wValue;
			prog.len = *len;
			memcpy(prog.buf, *buf, *len);
			if(usbdfu_state == STATE_DFU_IDLE)
				flash_unlock();
			usbdfu_state = STATE_DFU_DNLOAD_SYNC;
			return 1;
		}
	case DFU_CLRSTATUS:
		/* Clear error and return to dfuIDLE */
		if (usbdfu_state == STATE_DFU_ERROR)
			usbdfu_state = STATE_DFU_IDLE;
		return 1;
	case DFU_ABORT:
		/* Abort returns to dfuIDLE state */
		usbdfu_state = STATE_DFU_IDLE;
		return 1;
	case DFU_UPLOAD:
		if ((usbdfu_state == STATE_DFU_IDLE) ||
			(usbdfu_state == STATE_DFU_UPLOAD_IDLE)) {
			usbdfu_state = STATE_DFU_UPLOAD_IDLE;
			uint32_t baseaddr = prog.addr + req->wValue * dfu_function.wTransferSize;
			uint32_t copy_size = MAX_ADDRESS - baseaddr;
			gpio_toggle(LED_BANK, LED);
			if (copy_size >= dfu_function.wTransferSize) {
				memcpy(*buf, (void*)baseaddr, dfu_function.wTransferSize);
			} else {
				memcpy(*buf, (void*)baseaddr, copy_size);
				*len = copy_size;
				usbdfu_state = STATE_APP_DETACH;
			}
		}
		return 1;
	case DFU_GETSTATUS: {
		uint32_t bwPollTimeout = 0; /* 24-bit integer in DFU class spec */

		(*buf)[0] = usbdfu_getstatus(&bwPollTimeout);
		(*buf)[1] = bwPollTimeout & 0xFF;
		(*buf)[2] = (bwPollTimeout >> 8) & 0xFF;
		(*buf)[3] = (bwPollTimeout >> 16) & 0xFF;
		(*buf)[4] = usbdfu_state;
		(*buf)[5] = 0; /* iString not used here */
		*len = 6;

		*complete = usbdfu_getstatus_complete;

		return 1;
		}
	case DFU_GETSTATE:
		/* Return state with no state transition */
		*buf[0] = usbdfu_state;
		*len = 1;
		return 1;
	}

	return 0;
}

static bool dfuDnloadStarted(void) {
	return (usbdfu_state == STATE_DFU_DNBUSY || usbdfu_state == STATE_DFU_UPLOAD_IDLE) ? 1 : 0;
}

static bool dfuDnloadDone(void) {
	return (usbdfu_state == STATE_DFU_MANIFEST_WAIT_RESET) ? 1 : 0;
}

static bool dfuUploadDone(void) {
	return (usbdfu_state == STATE_APP_DETACH) ? 1 : 0;
}

static bool checkUserCode(uint32_t usrAddr) {
	uint32_t sp = *(volatile uint32_t *) usrAddr;

	if ((sp & 0x2FFE0000) == 0x20000000) {
		return (1);
	} else {
		return (0);
	}
}

static void jump_to_app_if_valid(void)
{
	/* Boot the application if it's valid */
	if(checkUserCode(APP_ADDRESS)) {
		/* Set vector table base address */
		SCB_VTOR = APP_ADDRESS & 0x1FFFF;
		/* Initialise master stack pointer */
		asm volatile ("msr msp, %0"::"g"
					(*(volatile uint32_t*)APP_ADDRESS));
		/* Jump to application */
		(*(void(**)())(APP_ADDRESS + 4))();
	}
}

static void RCC_DeInit(void)
{
  /* Set HSION bit */
	SET_REG(&RCC_CR, GET_REG(&RCC_CR)     | 0x00000001);

  /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
	SET_REG(&RCC_CFGR, GET_REG(&RCC_CFGR) & 0xF8FF0000);
  
  /* Reset HSEON, CSSON and PLLON bits */
	SET_REG(&RCC_CR, GET_REG(&RCC_CR)     & 0xFEF6FFFF);

  /* Reset HSEBYP bit */
	SET_REG(&RCC_CR, GET_REG(&RCC_CR)     & 0xFFFBFFFF);

  /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
	SET_REG(&RCC_CR, GET_REG(&RCC_CR)     & 0xFF80FFFF);

  /* Disable all interrupts and clear pending bits  */
	SET_REG(&RCC_CIR, 0x009F0000);
}

int main(void)
{
	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);

	rcc_periph_clock_enable(LED_RCC_GPIO);
	gpio_set_mode(LED_BANK, GPIO_MODE_OUTPUT_50_MHZ,
						GPIO_CNF_OUTPUT_PUSHPULL, LED);

#ifdef	NO_USB_DISC_HW
	rcc_periph_clock_enable(RCC_GPIOA);
	gpio_clear(GPIOA, GPIO12);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
			GPIO_CNF_OUTPUT_OPENDRAIN, GPIO12);
	volatile uint32_t delay;
	for(delay=800000;delay;delay--);
#endif

	usbd_device *usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev, &config,
		usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));

	usbd_register_control_callback(usbd_dev,
			USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
			USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
			usbdfu_control_request);

#if !defined(NO_USB_DISC_HW) && !defined(NO_USB_DISC)
	rcc_periph_clock_enable(USB_DISC_RCC_GPIO);
	gpio_clear(USB_DISC_BANK, USB_DISC);
	gpio_set_mode(USB_DISC_BANK, GPIO_MODE_OUTPUT_50_MHZ,
			GPIO_CNF_OUTPUT_OPENDRAIN, USB_DISC);
#endif

	bool no_user_jump = !checkUserCode(APP_ADDRESS);
  
	int delay_count = 0;
	
#define DELAY 400000 // wait to be catched by upgrade program
	while ((delay_count++ < 1) || no_user_jump) {
		gpio_set(LED_BANK, LED);
		for (int i=0; i<DELAY; i++) {
			usbd_poll(usbd_dev);
			if(i== (DELAY/2))
				gpio_clear(LED_BANK, LED);
			if(dfuDnloadStarted()) {
				gpio_clear(LED_BANK, LED);
				while(!dfuDnloadDone() && !dfuUploadDone()) {
					usbd_poll(usbd_dev);
				}
				// wait for last status request
				while((GET_REG(&OTG_FS_DIEPCTL0) & OTG_DIEPCTL0_NAKSTS) != OTG_DIEPCTL0_NAKSTS);
				break;
			}
		}
	}

#if !defined(NO_USB_DISC_HW) && !defined(NO_USB_DISC)
	gpio_set(USB_DISC_BANK, USB_DISC); // disconnect USB from host
#endif
	RCC_DeInit();
	jump_to_app_if_valid();
}
