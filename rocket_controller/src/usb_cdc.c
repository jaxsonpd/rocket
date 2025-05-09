/**
 * @file usb_cdc.c
 * @author Jack Duignan (JackpDuignan@gmail.com)
 * @date 2025-05-10
 * @brief Implementation for the USB CDC driver
 */


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>

#include "usb_cdc_desc.h"
#include "usb_cdc.h"

/* Buffer to be used for control requests. */
static uint8_t usbd_control_buffer[128];

/// The usb device for use in the cdc class
static usbd_device* usbd_device_cdc;

static CdcRxCB_t rx_callback = NULL;

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
        uint16_t* len, void (**complete)(usbd_device* usbd_dev, struct usb_setup_data* req));

static enum usbd_request_return_codes cdcacm_control_request(usbd_device* usbd_dev, struct usb_setup_data* req, uint8_t** buf,
    uint16_t* len, void (**complete)(usbd_device* usbd_dev, struct usb_setup_data* req)) {
    (void)complete;
    (void)buf;
    (void)usbd_dev;

    switch (req->bRequest) {
    case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
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
    }
    return USBD_REQ_NOTSUPP;
}

static void cdcacm_data_rx_cb(usbd_device* usbd_dev, uint8_t ep) {
    (void)ep;
    (void)usbd_dev;

    char buf[64];
    int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, 64);

    if (len) {
        if (rx_callback != NULL) {
            rx_callback(buf, len);
        }
        usbd_ep_write_packet(usbd_dev, 0x82, buf, len);
    }
}

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
    usbd_device_cdc = usbd_init(&st_usbfs_v1_usb_driver, &dev, &config, usb_strings, NUM_USB_STRINGS, usbd_control_buffer, sizeof(usbd_control_buffer));
    return usbd_register_set_config_callback(usbd_device_cdc, cdcacm_set_config);
}

void usb_cdc_add_rx_cb(CdcRxCB_t callback) {
    rx_callback = callback;
}

void usb_cdc_poll(void) {
    usbd_poll(usbd_device_cdc);
}

uint16_t usb_cdc_write(char *buf, uint16_t len) {
    return usbd_ep_write_packet(usbd_device_cdc, 0x82, buf, len);
} 