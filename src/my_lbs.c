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

 /** @brief LED Characteristic UUID. */
#define BT_UUID_LBS_MYSENSOR_VAL \
 BT_UUID_128_ENCODE(0x00001526, 0x1212, 0xefde, 0x1523, 0x785feabcd123)


 

 
#define BT_UUID_LBS BT_UUID_DECLARE_128(BT_UUID_LBS_VAL)
#define BT_UUID_LBS_BUTTON BT_UUID_DECLARE_128(BT_UUID_LBS_BUTTON_VAL)
#define BT_UUID_LBS_LED BT_UUID_DECLARE_128(BT_UUID_LBS_LED_VAL)
 #define BT_UUID_LBS_MYSENSOR       BT_UUID_DECLARE_128(BT_UUID_LBS_MYSENSOR_VAL)


static uint8_t button_state = 0; //0: not pressed, 1: pressed
static uint16_t button_ccc_indicate_enable = 0;
static uint16_t mysensor_ccc_indicate_enable = 0;

static struct bt_gatt_indicate_params button_indicate_param;

static ssize_t write_led(struct bt_conn *conn,\
 const struct bt_gatt_attr *attr,\
 const void *buf,\
 uint16_t len, uint16_t offset, uint8_t flags);

 static ssize_t read_button(struct bt_conn *conn, const struct bt_gatt_attr *attr,\
 void *buf,\
 uint16_t len,\
 uint16_t offset);

  static ssize_t read_mysensor(struct bt_conn *conn, const struct bt_gatt_attr *attr,\
 void *buf,\
 uint16_t len,\
 uint16_t offset);


 static void mylbs_buttton_ccc_changed( const struct bt_gatt_attr *attr,
 uint16_t value);

static void mylbs_mysensor_ccc_changed( const struct bt_gatt_attr *attr,
 uint16_t value);
 
// register the GATT service
BT_GATT_SERVICE_DEFINE(my_lbs_svc,
BT_GATT_PRIMARY_SERVICE(BT_UUID_LBS),
/* STEP 3 - Create and add the Button characteristic */ 
	BT_GATT_CHARACTERISTIC(BT_UUID_LBS_BUTTON,
			       BT_GATT_CHRC_READ|BT_GATT_CHRC_INDICATE,
			       BT_GATT_PERM_READ, read_button, NULL,
			       &button_state),
    BT_GATT_CCC(mylbs_buttton_ccc_changed, BT_GATT_PERM_READ|BT_GATT_PERM_WRITE),

 /* STEP 4 - Create and add the LED characteristic. */ 
 	BT_GATT_CHARACTERISTIC(BT_UUID_LBS_LED,
                            BT_GATT_CHRC_WRITE,
                            BT_GATT_PERM_WRITE,
                        NULL, write_led, NULL),

        BT_GATT_CHARACTERISTIC(BT_UUID_LBS_MYSENSOR,
                               BT_GATT_CHRC_NOTIFY,
                               BT_GATT_PERM_NONE, NULL, NULL,
                               NULL),
        BT_GATT_CCC(mylbs_mysensor_ccc_changed, BT_GATT_PERM_READ|BT_GATT_PERM_WRITE)   
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


static ssize_t read_mysensor(struct bt_conn *conn, const struct bt_gatt_attr *attr,
 void *buf,
 uint16_t len,
 uint16_t offset)
 {
       LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle, (void *)conn);
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


 static void mylbs_buttton_ccc_changed( const struct bt_gatt_attr *attr,
 uint16_t value)
 {
     LOG_DBG("Button CCC changed: %u", value);

     button_ccc_indicate_enable= (value && BT_GATT_CCC_INDICATE) ? 1 : 0;
    //  if (lbs_cb.button_cb) {
    //      lbs_cb.button_cb(value);
    //  }
 }

 static void mylbs_mysensor_ccc_changed( const struct bt_gatt_attr *attr,
 uint16_t value){
     LOG_DBG("MySensor CCC changed: %u", value);

     mysensor_ccc_indicate_enable= (value && BT_GATT_CCC_NOTIFY) ? 1 : 0;
     //  if (lbs_cb.mysensor_cb) {
     //      lbs_cb.mysensor_cb(value);
     //  }
 }
 void indicate_cb(struct bt_conn *conn,  struct bt_gatt_indicate_params *params,uint8_t err)
 {
     LOG_DBG("Indicate  call back %d\r\n",err);
 }

  void notify_cb(struct bt_conn *conn,  struct bt_gatt_notify_params *params,uint8_t err)
 {
     LOG_DBG("Notify call back %d\r\n",err);
 }

 int my_lbs_send_button_state_indicate(bool button_state)
{
    if (!button_ccc_indicate_enable) {
    return -EACCES;
    }  /* STEP 5.2 - Populate the indication */  

        // button_indicate_param.attr = &my_lbs_svc.attrs[2];
        button_indicate_param.uuid = BT_UUID_LBS_BUTTON;
        button_indicate_param.func = indicate_cb;
        button_indicate_param.destroy =NULL;
        button_indicate_param.data = &button_state;
        button_indicate_param.len = sizeof(button_state);

     return bt_gatt_indicate(NULL, &button_indicate_param);
}


int my_lbs_send_sensor_notify(uint32_t sensor_value)
{
 if (!mysensor_ccc_indicate_enable) {
 return -EACCES;
 }  
//  return bt_gatt_notify(NULL, &my_lbs_svc.attrs[7], 
//  &sensor_value,
//  sizeof(sensor_value));

    	struct bt_gatt_notify_params params;

	    memset(&params, 0, sizeof(params));
        params.uuid = BT_UUID_LBS_MYSENSOR;
        params.func = notify_cb;
        // params.destroy = NULL;
        params.data = &sensor_value;
        params.len = sizeof(sensor_value);
        return bt_gatt_notify_cb(NULL, &params);

}
