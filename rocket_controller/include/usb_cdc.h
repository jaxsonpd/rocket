/**
 * @file usb_cdc.h
 * @author Jack Duignan (JackpDuignan@gmail.com)
 * @date 2025-05-10
 * @brief Declarations for the USB CDC device driver
 *
 * Example Rx call back:
 *
 * static void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
 * {
 *	 (void)ep;
 *	 (void)usbd_dev;
 *
 *	 char buf[64];
 *	 int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, 64);
 *
 *	 if (len) {
 *      if (buf[0] == 'h') {
 *            gpio_toggle(GPIOC, GPIO13);
 *      }
 * 	    usbd_ep_write_packet(usbd_dev, 0x82, buf, len);
 *   }
 * }
 */


#ifndef USB_CDC_H
#define USB_CDC_H


#include <stdint.h>
#include <stdbool.h>

typedef void (*CdcRxCB_t)(char* buf, uint16_t len);

/**
 * @brief Initialise the USB CDC device, 48MHz usb clock must be enabled first
 *
 * @return 0 if successful
 */
int usb_cdc_init(void);

/** 
 * @brief Check if usb device is connected
 * 
 * @return true if connected
 */
bool usb_cdc_connected(void);

uint16_t usb_cdc_avail(void);
int usb_cdc_recv_byte(void);
void usb_cdc_send_byte(uint8_t ch);
void usb_cdc_send_strn(const char *str, size_t len);



bool usb_cdc_ready(void);


#endif // USB_CDC_H