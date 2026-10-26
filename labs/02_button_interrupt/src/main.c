/*
 * Lab 02: Button Inputs & Interrupts
 * Board: Seeed Studio XIAO ESP32C6
 *
 * Description:
 * Demonstrates event-driven GPIO handling using hardware interrupts:
 * 1. Configures pin D1 (GPIO 1) as an input with internal pull-up.
 * 2. Configures falling-edge interrupt (triggered when pin is pulled to GND).
 * 3. Registers an ISR callback function via gpio_init_callback / gpio_add_callback.
 * 4. Toggles onboard yellow LED (GPIO 15) instantly inside the callback.
 * 5. Demonstrates software debouncing using kernel uptime (k_uptime_get_32).
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lab02_button, LOG_LEVEL_INF);

/* User LED on XIAO ESP32C6 (GPIO 15) */
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/*
 * Use Pin D1 (GPIO 1) on XIAO ESP32C6 as our button input.
 * Connect a push button or temporary jumper wire between D1 and GND.
 */
#define BUTTON_PIN      1
#define DEBOUNCE_MS     250

static const struct device *const gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
static struct gpio_callback button_cb_data;
static uint32_t last_press_time = 0;
static uint32_t press_count = 0;

/*
 * Interrupt Service Routine (ISR) Callback:
 * Executed in ISR context whenever the button pin transitions from HIGH to LOW.
 * Keep work in ISR context minimal and non-blocking!
 */
static void button_pressed_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	uint32_t now = k_uptime_get_32();

	/* Simple software debounce filter */
	if ((now - last_press_time) < DEBOUNCE_MS) {
		return;
	}
	last_press_time = now;
	press_count++;

	/* Toggle the onboard LED */
	gpio_pin_toggle_dt(&led);

	LOG_INF(">>> Button Pressed! Total Count: %u (at %u ms)", press_count, now);
}

int main(void)
{
	int ret;

	LOG_INF("====================================================");
	LOG_INF("  Seeed Studio XIAO ESP32C6 - Lab 02 Button Interrupt");
	LOG_INF("====================================================");

	/* 1. Initialize LED */
	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED device %s is not ready!", led.port->name);
		return 0;
	}
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED: %d", ret);
		return 0;
	}

	/* 2. Check GPIO Controller Device */
	if (!device_is_ready(gpio_dev)) {
		LOG_ERR("GPIO controller %s is not ready!", gpio_dev->name);
		return 0;
	}

	/*
	 * 3. Configure Button Pin (GPIO 1 / D1):
	 * Input mode with internal pull-up enabled.
	 * Active state is LOW when grounded.
	 */
	LOG_INF("Configuring Button on D1 (GPIO %d) with Pull-Up...", BUTTON_PIN);
	ret = gpio_pin_configure(gpio_dev, BUTTON_PIN, GPIO_INPUT | GPIO_PULL_UP);
	if (ret < 0) {
		LOG_ERR("Failed to configure button pin: %d", ret);
		return 0;
	}

	/* 4. Configure Edge Interrupt (Falling Edge / High to Low) */
	ret = gpio_pin_interrupt_configure(gpio_dev, BUTTON_PIN, GPIO_INT_EDGE_FALLING);
	if (ret < 0) {
		LOG_ERR("Failed to configure interrupt: %d", ret);
		return 0;
	}

	/* 5. Initialize and register the callback handler */
	gpio_init_callback(&button_cb_data, button_pressed_callback, BIT(BUTTON_PIN));
	ret = gpio_add_callback(gpio_dev, &button_cb_data);
	if (ret < 0) {
		LOG_ERR("Failed to add callback: %d", ret);
		return 0;
	}

	LOG_INF("Interrupt configured successfully.");
	LOG_INF("-> Connect a wire between D1 (Pin 2) and GND to trigger!");

	/* Main thread can now idle; everything is event-driven by hardware interrupts */
	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
