#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include "my_lbs.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(my_lbs, LOG_LEVEL_DBG);

/************ custom service */

/** @brief LBS Service UUID. */
#define BT_UUID_LBS_VAL \
 BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123) 
/** @brief Button Characteristic UUID. */
#define BT_UUID_LBS_BUTTON_VAL \
 BT_UUID_128_ENCODE(0x00001524, 0x1212, 0xefde, 0x1523, 0x785feabcd123) 
/** @brief LED Characteristic UUID. */
#define BT_UUID_LBS_LED_VAL \
 BT_UUID_128_ENCODE(0x00001525, 0x1212, 0xefde, 0x1523, 0x785feabcd123) 
 
#define BT_UUID_LBS BT_UUID_DECLARE_128(BT_UUID_LBS_VAL)
#define BT_UUID_LBS_BUTTON BT_UUID_DECLARE_128(BT_UUID_LBS_BUTTON_VAL)
#define BT_UUID_LBS_LED BT_UUID_DECLARE_128(BT_UUID_LBS_LED_VAL)

static uint8_t button_state = 0; //0: not pressed, 1: pressed

static ssize_t write_led(struct bt_conn *conn,
 const struct bt_gatt_attr *attr,
 const void *buf,
 uint16_t len, uint16_t offset, uint8_t flags);
 static ssize_t read_button(struct bt_conn *conn, const struct bt_gatt_attr *attr,
 void *buf,
 uint16_t len,
 uint16_t offset);

// register the GATT service
BT_GATT_SERVICE_DEFINE(my_lbs_svc,
BT_GATT_PRIMARY_SERVICE(BT_UUID_LBS),
/* STEP 3 - Create and add the Button characteristic */ 
	BT_GATT_CHARACTERISTIC(BT_UUID_LBS_BUTTON,
			       BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ, read_button, NULL,
			       &button_state),

 /* STEP 4 - Create and add the LED characteristic. */ 
 	BT_GATT_CHARACTERISTIC(BT_UUID_LBS_LED,
 BT_GATT_CHRC_WRITE,
 BT_GATT_PERM_WRITE,
 NULL, write_led, NULL),
 );


static struct my_lbs_cb lbs_cb;
/******************************* */

void my_lbs_init (struct my_lbs_cb *cb)
{
        // Initialize the LBS service
        lbs_cb.led_cb = cb->led_cb;
        lbs_cb.button_cb = cb->button_cb;

}


static ssize_t read_button(struct bt_conn *conn, const struct bt_gatt_attr *attr,
 void *buf,
 uint16_t len,
 uint16_t offset)
{
 //get a pointer to button_state which is passed in the BT_GATT_CHARACTERISTIC() and stored in attr->user_data
 const char *value = attr->user_data; 
  LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle,
 (void *)conn);  
 if (lbs_cb.button_cb) {
    // Call the application callback function to update the get the current value of the button
    // button_state = lbs_cb.button_cb();
    if(button_state)
    {
        button_state = 0;
    }
    else
    {
        button_state = 1;
    }

    printk("Button state addr: %x\n",&button_state);
    printk("User data addr: %x\n",attr->user_data);
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
        sizeof(*value));
 } 
  return 0;
}


static ssize_t write_led(struct bt_conn *conn,
 const struct bt_gatt_attr *attr,
 const void *buf,
 uint16_t len, uint16_t offset, uint8_t flags)
{
 LOG_DBG("Attribute write, handle: %u, conn: %p", attr->handle,
 (void *)conn);  
 if (len != 1U) {
    LOG_DBG("Write led: Incorrect data length");
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
 } 
  if (offset != 0) {
    LOG_DBG("Write led: Incorrect data offset");
    return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
 } 
  if (lbs_cb.led_cb) {
    //Read the received value 
    uint8_t val = *((uint8_t *)buf);  
    if (val == 0x00 || val == 0x01) {
        //Call the application callback function to update the LED state
        lbs_cb.led_cb(val ? true : false);
    } else {
        LOG_DBG("Write led: Incorrect value");
        return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
    }
 }  return len;
}