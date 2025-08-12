#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>


#include "my_lbs.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define DEVCE_NAME             CONFIG_BT_DEVICE_NAME 
#define DEVCE_NAME_LEN         (sizeof(DEVCE_NAME) - 1 )
static struct k_work adv_work;  
// static int32_t conn

void led_callback(bool flat)
{


}

uint8_t button_callback()
{
    return 1;
}
        
static struct my_lbs_cb lbs_cb={
        .led_cb = led_callback,
        .button_cb = button_callback
};

static struct bt_le_adv_param adv_param = {
        .options = BT_LE_ADV_OPT_CONN|BT_LE_ADV_OPT_USE_NAME,
        .interval_min = BT_GAP_ADV_SLOW_INT_MIN,
        .interval_max = BT_GAP_ADV_SLOW_INT_MAX,
        .peer = NULL,
};
static const struct bt_data ad[] = {
        BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
        // BT_DATA_BYTES(BT_DATA_NAME_COMPLETE, 'Z', 'e', 'p', 'h', 'y', 'r', ' ', 'B', 'l', 'u', 'e', 't', 'o', 'o', 't', 'h')
       // BT_DATA(BT_DATA_NAME_COMPLETE, DEVCE_NAME, DEVCE_NAME_LEN)
};

static const struct bt_data sd[] = {
        BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x18, 0x0F) // Generic Access Profile
};


static void adv_work_handler(struct k_work *work)
{
        int err;

        err = bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
        if (err) {
                LOG_ERR("Advertising failed to start (err %d)", err);
                return;
        }

        LOG_INF("Advertising successfully started");
}

static void advertising_start(void)
{
	k_work_submit(&adv_work);
}
static void recycled_cb(void)
{
	printk("Connection object available from previous conn. Disconnect is complete!\n");
	advertising_start();
}

static void on_connected(struct bt_conn *conn, uint8_t err)
{
        if (err) {
                LOG_ERR("Connection failed (err %d)", err);
                return;
        }

        LOG_INF("Connected");
}

static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
        LOG_INF("Disconnected (reason %d)", reason);
}

static struct bt_conn_cb conn_callbacks = {
        .recycled = recycled_cb,
        .connected = on_connected,
        .disconnected = on_disconnected,
        .le_param_req = NULL,
        .le_param_updated = NULL,
};



int main(void)
{
        // return 0;

        int err;

        my_lbs_init(&lbs_cb);
        
        bt_enable(NULL);
        if (err) {
                LOG_ERR("Bluetooth enable failed (err %d)", err);
                return err;
        }
        LOG_INF("Bluetooth initialized");
        err = bt_conn_cb_register(&conn_callbacks);
        if (err) {
                LOG_ERR("Failed to register connection callbacks (err %d)", err);
                return err;
        }

        k_work_init(&adv_work, adv_work_handler); // Initialize the work item with a handler
        
        advertising_start(); // Start advertising

        for(;;)
        {
                LOG_INF("Hello, World!");
                k_msleep(1000); // Sleep for 1000 milliseconds (1 second)
        }       
}
