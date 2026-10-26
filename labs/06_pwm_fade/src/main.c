/*
 * Lab 06: Hardware PWM & Breathing LED
 * Board: Seeed Studio XIAO ESP32C6
 *
 * Description:
 * Demonstrates Zephyr's Hardware PWM subsystem:
 * 1. Retrieves PWM specification from devicetree alias (pwm_led0).
 * 2. Checks pwm_is_ready_dt().
 * 3. Modulates the pulse width (duty cycle) in small incremental steps
 *    to create a smooth, organic "breathing" LED effect.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lab06_pwm, LOG_LEVEL_INF);

/* Devicetree PWM LED specification */
static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

#define STEP_SIZE_PCT   2       /* Increment/decrement by 2% each step */
#define STEP_DELAY_MS   25      /* Delay between brightness updates */

int main(void)
{
	uint32_t period = pwm_led0.period;
	uint32_t pulse = 0;
	int dir = 1; /* 1 = brightening, -1 = dimming */
	int pct = 0;
	int ret;

	LOG_INF("==========================================================");
	LOG_INF("  Seeed Studio XIAO ESP32C6 - Lab 06 PWM Breathing LED    ");
	LOG_INF("==========================================================");

	if (!pwm_is_ready_dt(&pwm_led0)) {
		LOG_ERR("Error: PWM device %s is not ready!", pwm_led0.dev->name);
		return 0;
	}

	LOG_INF("PWM Device: %s, Channel: %d, Period: %u ns",
		pwm_led0.dev->name, pwm_led0.channel, period);
	LOG_INF("Starting breathing LED modulation...");

	while (1) {
		/* Calculate pulse width based on current percentage */
		pulse = (uint64_t)period * pct / 100;

		ret = pwm_set_pulse_dt(&pwm_led0, pulse);
		if (ret < 0) {
			LOG_ERR("pwm_set_pulse_dt failed: %d", ret);
			return 0;
		}

		/* Update percentage and change direction at extremes */
		pct += (dir * STEP_SIZE_PCT);
		if (pct >= 100) {
			pct = 100;
			dir = -1;
		} else if (pct <= 0) {
			pct = 0;
			dir = 1;
		}

		k_msleep(STEP_DELAY_MS);
	}

	return 0;
}
