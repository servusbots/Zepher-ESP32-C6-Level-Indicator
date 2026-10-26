/*
 * Lab 08: Bluetooth Low Energy (BLE 5.3) Beacon
 * Board: Seeed Studio XIAO ESP32C6
 *
 * Description:
 * Demonstrates the industry-leading Zephyr Bluetooth LE stack:
 * 1. Initializes the BLE controller and host using bt_enable().
 * 2. Assembles GAP advertising data packets (Flags, Device Name, and Manufacturer Data).
 * 3. Starts non-connectable advertising (Beacon mode).
 * 4. Periodically updates dynamic telemetry payload (broadcast counter and uptime)
 *    so any smartphone scanning via 'nRF Connect' sees live updates!
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>

LOG_MODULE_REGISTER(lab08_ble, LOG_LEVEL_INF);

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* Company identifier (0xFFFF = Test ID) and dynamic beacon payload */
#pragma pack(push, 1)
struct adv_payload {
	uint16_t company_id;
	uint8_t  battery_pct;
	uint16_t temperature_c_x10;
	uint32_t packet_counter;
};
#pragma pack(pop)

static struct adv_payload beacon_data = {
	.company_id = 0xFFFF,
	.battery_pct = 95,
	.temperature_c_x10 = 245, /* 24.5 C */
	.packet_counter = 0
};

/* Advertising Data Packet */
static struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, &beacon_data, sizeof(beacon_data)),
};

int main(void)
{
	int err;

	LOG_INF("==========================================================");
	LOG_INF("  Seeed Studio XIAO ESP32C6 - Lab 08 BLE Beacon           ");
	LOG_INF("==========================================================");

	/* Initialize LED */
	if (gpio_is_ready_dt(&led)) {
		gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	}

	/* 1. Enable Bluetooth Host & Controller */
	LOG_INF("Initializing Zephyr Bluetooth LE subsystem...");
	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth initialization failed (err %d)", err);
		return 0;
	}
	LOG_INF("Bluetooth initialized successfully.");

	/* 2. Start Advertising */
	/*
	 * Advertising parameters:
	 *   BT_LE_ADV_NCONN: Non-connectable, undirected advertising
	 *   Interval: 100ms min, 150ms max
	 */
	err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return 0;
	}

	LOG_INF(">>> BLE Advertising Active! Name: '%s'", CONFIG_BT_DEVICE_NAME);
	LOG_INF("Open the 'nRF Connect' app on your phone to scan for this device.");

	/* Loop: Update telemetry and toggle LED */
	while (1) {
		k_msleep(2000);

		beacon_data.packet_counter++;
		/* Simulate small battery/temp variations */
		beacon_data.temperature_c_x10 = 240 + (beacon_data.packet_counter % 20);

		/* Update live advertising payload */
		err = bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
		if (err) {
			LOG_WRN("Failed to update advertising data (err %d)", err);
		}

		/* Flash LED to indicate beacon broadcast pulse */
		gpio_pin_set_dt(&led, 1);
		k_msleep(50);
		gpio_pin_set_dt(&led, 0);

		LOG_INF("Broadcasted Beacon Frame #%u (Temp: %d.%d C)",
			beacon_data.packet_counter,
			beacon_data.temperature_c_x10 / 10,
			beacon_data.temperature_c_x10 % 10);
	}

	return 0;
}
