/*
 * Lab 05: Kernel Timers & System Workqueues
 * Board: Seeed Studio XIAO ESP32C6
 *
 * Description:
 * Demonstrates bottom-half offloading from ISR/timer context to thread context:
 * 1. A periodic kernel timer (k_timer) triggers every 2 seconds.
 * 2. The timer expiry callback runs in ISR/softirq context.
 * 3. Instead of performing heavy operations in ISR context, the timer calls
 *    k_work_submit(&heavy_work) to enqueue work for the System Workqueue thread.
 * 4. The work item executes in thread context, where it safely toggles the LED
 *    and simulates a time-consuming measurement.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lab05_workqueue, LOG_LEVEL_INF);

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* Forward declaration of work item callback */
static void work_handler(struct k_work *work);

/* Statically initialize a work item */
K_WORK_DEFINE(sample_work_item, work_handler);

/* Forward declaration of timer expiry callback */
static void timer_expiry_handler(struct k_timer *timer);

/* Statically initialize a timer */
K_TIMER_DEFINE(sample_timer, timer_expiry_handler, NULL);

static uint32_t sample_count = 0;

/*
 * Timer Expiry Callback:
 * Runs in ISR context! Must be lightning fast and NEVER block.
 */
static void timer_expiry_handler(struct k_timer *timer)
{
	ARG_UNUSED(timer);

	/*
	 * Offload processing to the System Workqueue thread.
	 * This returns immediately without blocking the timer!
	 */
	k_work_submit(&sample_work_item);
}

/*
 * Workqueue Handler:
 * Runs in the System Workqueue THREAD context.
 * Safe to block, wait for semaphores, or perform I/O!
 */
static void work_handler(struct k_work *work)
{
	ARG_UNUSED(work);
	sample_count++;

	LOG_INF("[Workqueue] Executing Work Item #%u in thread context (TID: %p)",
		sample_count, k_current_get());

	/* Turn on LED */
	gpio_pin_set_dt(&led, 1);

	/* Simulate heavy computational work / slow I/O by sleeping 200ms */
	k_msleep(200);

	/* Turn off LED */
	gpio_pin_set_dt(&led, 0);

	LOG_INF("[Workqueue] Work Item #%u completed.", sample_count);
}

int main(void)
{
	LOG_INF("==========================================================");
	LOG_INF("  Seeed Studio XIAO ESP32C6 - Lab 05 Timers & Workqueues  ");
	LOG_INF("==========================================================");

	/* Initialize LED */
	if (gpio_is_ready_dt(&led)) {
		gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	}

	LOG_INF("Starting periodic timer: Fires every 2000 ms...");

	/*
	 * Start the timer:
	 *   - initial delay: 1000 ms
	 *   - recurring period: 2000 ms
	 */
	k_timer_start(&sample_timer, K_MSEC(1000), K_MSEC(2000));

	/* Main thread can idle */
	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
