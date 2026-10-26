/*
 * Lab 03: USB-Serial Logging & Interactive Shell
 * Board: Seeed Studio XIAO ESP32C6
 *
 * Description:
 * Demonstrates Zephyr's powerful interactive CLI shell and logging subsystems:
 * 1. Log modules, log filtering levels, and timestamped color output.
 * 2. Registration of custom CLI commands using SHELL_CMD_REGISTER and SHELL_STATIC_SUBCMD_SET_CREATE.
 * 3. Commands:
 *    - 'board_info': Prints chip architecture, operating frequency, and system uptime.
 *    - 'led <on|off|toggle>': Directly manipulates the onboard user LED from your terminal.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/version.h>

LOG_MODULE_REGISTER(lab03_shell, LOG_LEVEL_DBG);

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* Handler for 'board_info' shell command */
static int cmd_board_info(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(sh, "========================================");
	shell_print(sh, " Board:      Seeed Studio XIAO ESP32C6");
	shell_print(sh, " SoC:        Espressif ESP32-C6 (RISC-V)");
	shell_print(sh, " Core Clock: %u MHz", sys_clock_hw_cycles_per_sec() / 1000000);
	shell_print(sh, " OS Version: Zephyr %s", KERNEL_VERSION_STRING);
	shell_print(sh, " Uptime:     %u ms", k_uptime_get_32());
	shell_print(sh, "========================================");

	return 0;
}

/* Handler for 'led on' */
static int cmd_led_on(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	gpio_pin_set_dt(&led, 1);
	shell_print(sh, "LED turned ON");
	return 0;
}

/* Handler for 'led off' */
static int cmd_led_off(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	gpio_pin_set_dt(&led, 0);
	shell_print(sh, "LED turned OFF");
	return 0;
}

/* Handler for 'led toggle' */
static int cmd_led_toggle(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	gpio_pin_toggle_dt(&led);
	shell_print(sh, "LED toggled");
	return 0;
}

/* Register subcommands for 'led' */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_led,
	SHELL_CMD(on, NULL, "Turn onboard LED ON", cmd_led_on),
	SHELL_CMD(off, NULL, "Turn onboard LED OFF", cmd_led_off),
	SHELL_CMD(toggle, NULL, "Toggle onboard LED state", cmd_led_toggle),
	SHELL_SUBCMD_SET_END
);

/* Register top-level shell commands */
SHELL_CMD_REGISTER(board_info, NULL, "Show board metadata and system uptime", cmd_board_info);
SHELL_CMD_REGISTER(led, &sub_led, "Control onboard LED (on, off, toggle)", NULL);

int main(void)
{
	LOG_INF("====================================================");
	LOG_INF("  Seeed Studio XIAO ESP32C6 - Lab 03 Logging & Shell");
	LOG_INF("====================================================");

	/* Initialize LED */
	if (gpio_is_ready_dt(&led)) {
		gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	}

	LOG_INF("Zephyr Shell ready! Open serial monitor to interact.");
	LOG_INF("Type 'help' in the shell prompt to see available commands.");

	/* Demonstrate logging levels */
	LOG_DBG("This is a DEBUG message (LOG_DBG)");
	LOG_INF("This is an INFO message (LOG_INF)");
	LOG_WRN("This is a WARNING message (LOG_WRN)");

	return 0;
}
