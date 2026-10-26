/*
 * Lab 04: RTOS Multithreading & Synchronization
 * Board: Seeed Studio XIAO ESP32C6
 *
 * Description:
 * Demonstrates the core primitives of Zephyr's preemptive multi-threaded kernel:
 * 1. Two concurrent threads created statically via K_THREAD_DEFINE:
 *    - Producer Thread: Generates simulated sensor readings and sends them via k_msgq.
 *    - Consumer Thread: Waits for data from k_msgq, logs the readings, and pulses the LED.
 * 2. Synchronization using a Binary Semaphore (k_sem) to signal high-priority events.
 * 3. Demonstrates thread priority: Lower numerical value = Higher priority in Zephyr.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lab04_rtos, LOG_LEVEL_INF);

/* User LED on XIAO ESP32C6 */
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* Thread Stack Sizes and Priorities */
#define STACK_SIZE      1024
#define PRODUCER_PRIO   7
#define CONSUMER_PRIO   6   /* Consumer has higher priority (runs immediately when data arrives) */

/* Data packet exchanged between threads */
struct sensor_packet {
	uint32_t sequence;
	int16_t temperature_c_x10;  /* e.g. 245 = 24.5 C */
	uint32_t timestamp_ms;
};

/*
 * Message Queue:
 * Allocates an in-memory FIFO queue buffer for 10 sensor_packet items.
 */
K_MSGQ_DEFINE(sensor_msgq, sizeof(struct sensor_packet), 10, 4);

/*
 * Semaphore:
 * Used to signal an alert or emergency state across threads.
 */
K_SEM_DEFINE(alert_sem, 0, 1);

/* Forward Declarations of Thread Entry Functions */
void producer_thread_entry(void *p1, void *p2, void *p3);
void consumer_thread_entry(void *p1, void *p2, void *p3);

/* Statically Define and Auto-Start the Threads */
K_THREAD_DEFINE(producer_tid, STACK_SIZE,
		producer_thread_entry, NULL, NULL, NULL,
		PRODUCER_PRIO, 0, 0);

K_THREAD_DEFINE(consumer_tid, STACK_SIZE,
		consumer_thread_entry, NULL, NULL, NULL,
		CONSUMER_PRIO, 0, 0);

/* Producer Thread: Generates data every 1 second */
void producer_thread_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	struct sensor_packet packet;
	uint32_t seq = 0;
	int16_t temp = 220; /* 22.0 C */

	LOG_INF("[Producer] Thread started (Priority: %d)", PRODUCER_PRIO);

	while (1) {
		k_msleep(1000);

		packet.sequence = ++seq;
		/* Simulate small temperature fluctuations */
		packet.temperature_c_x10 = temp + (seq % 5) * 5;
		packet.timestamp_ms = k_uptime_get_32();

		LOG_INF("[Producer] Generated Packet #%u (Temp: %d.%d C)",
			packet.sequence,
			packet.temperature_c_x10 / 10,
			packet.temperature_c_x10 % 10);

		/* Send packet into message queue. Non-blocking (K_NO_WAIT) */
		if (k_msgq_put(&sensor_msgq, &packet, K_NO_WAIT) != 0) {
			LOG_WRN("[Producer] Message queue is full! Dropped packet #%u", seq);
		}

		/* Trigger an alert every 5th packet */
		if (seq % 5 == 0) {
			LOG_INF("[Producer] Triggering high-priority alert semaphore!");
			k_sem_give(&alert_sem);
		}
	}
}

/* Consumer Thread: Consumes data from queue and processes alerts */
void consumer_thread_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	struct sensor_packet packet;

	LOG_INF("[Consumer] Thread started (Priority: %d)", CONSUMER_PRIO);

	while (1) {
		/*
		 * Block waiting for a packet from the message queue.
		 * Sleeps until a packet is put into the queue (K_FOREVER).
		 */
		if (k_msgq_get(&sensor_msgq, &packet, K_MSEC(500)) == 0) {
			LOG_INF("[Consumer] Received Packet #%u from Queue: Temp = %d.%d C",
				packet.sequence,
				packet.temperature_c_x10 / 10,
				packet.temperature_c_x10 % 10);

			/* Pulse LED briefly to indicate packet received */
			gpio_pin_set_dt(&led, 1);
			k_msleep(50);
			gpio_pin_set_dt(&led, 0);
		}

		/* Check if high-priority alert semaphore was given */
		if (k_sem_take(&alert_sem, K_NO_WAIT) == 0) {
			LOG_WRN("[Consumer] *** ALERT PROCESSED: Rapid Blink Sequence ***");
			for (int i = 0; i < 6; i++) {
				gpio_pin_toggle_dt(&led);
				k_msleep(100);
			}
			gpio_pin_set_dt(&led, 0);
		}
	}
}

int main(void)
{
	LOG_INF("====================================================");
	LOG_INF("  Seeed Studio XIAO ESP32C6 - Lab 04 Multithreading ");
	LOG_INF("====================================================");

	/* Initialize LED */
	if (gpio_is_ready_dt(&led)) {
		gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	}

	LOG_INF("Main thread initialization complete. Handing over to threads...");
	return 0;
}
