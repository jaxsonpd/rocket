/**
 * @file usb_cdc.c
 * @author Jack Duignan (JackpDuignan@gmail.com)
 * @date 2025-05-10
 * @brief Implementation for the USB CDC driver
 * 
 * @cite dhylands https://github.com/dhylands/libopencm3-usb-serial/blob/master/usb.c#L361
 */


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/nvic.h>

#include "CBUF.h"

#include "usb_cdc_desc.h"
#include "usb_cdc.h"

typedef struct {
	volatile	uint16_t	m_get_idx;
	volatile	uint16_t	m_put_idx;
				uint8_t		m_entry[1024];	// Size must be a power of 2
} buf_t;

/// Buffers to store data to send and receive
static buf_t	usb_serial_rx_buf;
static buf_t	usb_serial_tx_buf;

/// true if need to send an empty usb packet to flush system 
static bool   	usb_serial_need_empty_tx = false;

/// Flag high when usb is connected
static bool g_usbd_is_connected = false;

/// Buffer to be used for control requests
static uint8_t g_usbd_control_buffer[128];

/// The usb device for use in the cdc class
static usbd_device* g_usbd_device_cdc = NULL;

static const struct usb_cdc_line_coding line_coding = {
	.dwDTERate = 115200,
	.bCharFormat = USB_CDC_1_STOP_BITS,
	.bParityType = USB_CDC_NO_PARITY,
	.bDataBits = 0x08
};

/**
 * @brief Process the control requests from the usb end point
 * @param usbd_dev the usb device
 * @param req the setup request to process
 * @param buf the usb buf
 * @param len the len of the buffer
 *
 * @return the result as a return code
 */
static enum usbd_request_return_codes cdcacm_control_request(usbd_device* usbd_dev, struct usb_setup_data* req, uint8_t** buf,
    uint16_t* len, void (**complete)(usbd_device* usbd_dev, struct usb_setup_data* req)) {
    (void)complete;
    (void)buf;
    (void)usbd_dev;

    switch (req->bRequest) {
    case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
        uint16_t rtsdtr = req->wValue;	// DTR is bit 0, RTS is bit 1
        g_usbd_is_connected = rtsdtr & 1;

        /*
         * This Linux cdc_acm driver requires this to be implemented
         * even though it's optional in the CDC spec, and we don't
         * advertise it in the ACM functional descriptor.
         */
        char local_buf[10];
        struct usb_cdc_notification* notif = (void*)local_buf;

        /* We echo signals back to host as notification. */
        notif->bmRequestType = 0xA1;
        notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
        notif->wValue = 0;
        notif->wIndex = 0;
        notif->wLength = 2;
        local_buf[8] = req->wValue & 3;
        local_buf[9] = 0;
        // usbd_ep_write_packet(0x83, buf, 10);
        return USBD_REQ_HANDLED;
    }
    case USB_CDC_REQ_SET_LINE_CODING:
        if (*len < sizeof(struct usb_cdc_line_coding))
            return USBD_REQ_NOTSUPP;
        return USBD_REQ_HANDLED;
    case USB_CDC_REQ_GET_LINE_CODING:
        *buf = (uint8_t *)&line_coding;
        return USBD_REQ_HANDLED;
    }
    return USBD_REQ_NOTSUPP;
}

/** 
 * @brief Called when new usb data is received must use usb poll for this to occur
 * @param usbd_dev the device that generated the interrupt
 * @param ep the endpoint that generated the interrupt
 * 
 */
static void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep) {
	uint16_t space = CBUF_ContigSpace(usb_serial_rx_buf);
	uint16_t len;
	if (space >= 64) {
		// We can read directly into our buffer
		len = usbd_ep_read_packet(usbd_dev, ep, 
								  CBUF_GetPushEntryPtr(usb_serial_rx_buf), 64);
		CBUF_AdvancePushIdxBy(usb_serial_rx_buf, len);
	} else {
		// Do it character by character. This covers 2 situations:
		// 1 - We're near the end of the buffer (so free space isn't contiguous)
		// 2 - We don't have much free space left.
		char buf[64];
		len = usbd_ep_read_packet(usbd_dev, ep, buf, 64);
		for (int i = 0; i < len; i++) {
			// If the Rx buffer fills, then we drop the new data.
			if (CBUF_IsFull(usb_serial_rx_buf)) {
				return;
			}
			CBUF_Push(usb_serial_rx_buf, buf[i]);
		}
	}
}


/** 
 * @brief USB on the go interrupt handler
 * 
 */
void usb_lp_can_rx0_isr(void) {
	if (g_usbd_device_cdc) {
		usbd_poll(g_usbd_device_cdc);
	}
}

static void cdcacm_sof_callback(void) {
	if (!g_usbd_is_connected) {
		// Host isn't connected - nothing to do.
		return;
	}

	uint16_t len = CBUF_ContigLen(usb_serial_tx_buf);
	if (len == 0 && !usb_serial_need_empty_tx) {
		// Nothing to do.
		return;
	}
	if (len > 64) {
		len = 64;
	}
	uint8_t *pop_ptr = CBUF_GetPopEntryPtr(usb_serial_tx_buf);
	uint16_t sent = usbd_ep_write_packet(g_usbd_device_cdc, 0x82, pop_ptr, len);

	// If we just sent a packet of 64 bytes. If we get called again and
	// there is no more data to send, then we need to send a zero byte
	// packet to indicate to the host to release the data it has buffered.
	usb_serial_need_empty_tx = (sent == 64);
	CBUF_AdvancePopIdxBy(usb_serial_tx_buf, sent);
}

/** 
 * @brief Set config handler, called when the usb is connected
 * @param usbd_dev the device that generated the interrupt
 * @param ep the endpoint that generated the interrupt
 * 
 */
static void cdcacm_set_config(usbd_device* usbd_dev, uint16_t wValue) {
    (void)wValue;
    (void)usbd_dev;

    usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_rx_cb);
    usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_BULK, 64, NULL);
    usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

    usbd_register_control_callback(
        usbd_dev,
        USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
        USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
        cdcacm_control_request);
}



int usb_cdc_init(void) {
    g_usbd_device_cdc = usbd_init(&st_usbfs_v1_usb_driver, &dev, &config, 
                                usb_strings, NUM_USB_STRINGS, 
                                g_usbd_control_buffer, sizeof(g_usbd_control_buffer));
    usbd_register_set_config_callback(g_usbd_device_cdc, cdcacm_set_config);
    usbd_register_sof_callback(g_usbd_device_cdc, cdcacm_sof_callback);
    nvic_enable_irq(NVIC_USB_LP_CAN_RX0_IRQ);
    nvic_set_priority(NVIC_USB_LP_CAN_RX0_IRQ, 1);

    return 0;

}

bool usb_cdc_connected(void) {
    return g_usbd_is_connected;
}

uint16_t usb_cdc_avail(void) {
	return CBUF_Len(usb_serial_rx_buf);
}

int usb_cdc_recv_byte(void) {
	if (CBUF_IsEmpty(usb_serial_rx_buf)) {
		return -1;
	}
	return CBUF_Pop(usb_serial_rx_buf);
}

void usb_cdc_send_byte(uint8_t ch) {
	if (!CBUF_IsFull(usb_serial_tx_buf)) {
		CBUF_Push(usb_serial_tx_buf, ch);
	}
}

void usb_cdc_send_strn(const char *str, size_t len) {
	for (const char *end = str + len; str < end; str++) {
		usb_cdc_send_byte(*str);
	}
}

static int usb_putc(void *out_param, int ch) {
	(void)out_param;
	if (ch == '\n') {
		usb_cdc_send_byte('\r');
	}
	usb_cdc_send_byte(ch);
	return 1;
}

// void usb_cdc_printf(const char *fmt, ...) {
// 	va_list args;
// 	va_start(args, fmt);
// 	vStrXPrintf(usb_putc, NULL, fmt, args);
// 	va_end(args);
// }