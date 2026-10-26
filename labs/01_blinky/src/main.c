/*
 * Lab 01: Blinky & Devicetree Fundamentals
 * Board: Seeed Studio XIAO ESP32C6
 *
 * Description:
 * Demonstrates the core Zephyr Devicetree hardware abstraction:
 * 1. Grabbing the hardware specification struct for 'led0' using DT_ALIAS.
 * 2. Validating that the GPIO device driver is ready (gpio_is_ready_dt).
 * 3. Configuring the pin as an active output (gpio_pin_configure_dt).
 * 4. Toggling the pin state and sleeping the kernel thread (k_msleep).
 *
 * Note on XIAO ESP32C6:
 * The onboard user LED is connected to GPIO15 with Active-LOW polarity.
 * Zephyr's Devicetree handles the active-low inversion automatically!
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lab01_blinky, LOG_LEVEL_INF);

/* 1000 milliseconds = 1 Hz blink rate */
#define SLEEP_TIME_MS   1000

/*
 * Retrieve the devicetree node identifier for the alias 'led0'.
 * In boards/seeed/xiao_esp32c6/xiao_esp32c6_hpcore.dts:
 *   aliases { led0 = &yellow_led; };
 */
#define LED0_NODE DT_ALIAS(led0)

/*
 * Extract the GPIO specification from the devicetree node.
 * This encapsulates:
 *   - The device pointer (&gpio0)
 *   - The pin number (15)
 *   - Flags (GPIO_ACTIVE_LOW)
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
	int ret;
	bool led_state = true;

	LOG_INF("============================================");
	LOG_INF("  Seeed Studio XIAO ESP32C6 - Lab 01 Blinky ");
	LOG_INF("============================================");

	/* Verify the hardware device is initialized and ready */
	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("Error: GPIO device %s is not ready!", led.port->name);
		return 0;
	}

	LOG_INF("Configuring LED on %s pin %d (active %s)...",
		led.port->name,
		led.pin,
		(led.dt_flags & GPIO_ACTIVE_LOW) ? "LOW" : "HIGH");

	/* Configure the GPIO pin as an output, initialized to active (ON) */
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Error configuring GPIO pin: %d", ret);
		return 0;
	}

	LOG_INF("Blinking LED every %d ms. Press Reset or disconnect to stop.", SLEEP_TIME_MS);

	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			LOG_ERR("Failed to toggle pin: %d", ret);
			return 0;
		}

		led_state = !led_state;
		LOG_INF("LED State: %s", led_state ? "ON" : "OFF");

		/* Put the current thread to sleep for the specified duration */
		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}
