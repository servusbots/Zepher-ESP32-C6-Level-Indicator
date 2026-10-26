#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define LED0_NODE       DT_ALIAS(led0)
#define RF_EN_NODE      DT_NODELABEL(rf_switch_en)
#define RF_SEL_NODE     DT_NODELABEL(rf_ant_sel)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec rf_en = GPIO_DT_SPEC_GET(RF_EN_NODE, gpios);
static const struct gpio_dt_spec rf_sel = GPIO_DT_SPEC_GET(RF_SEL_NODE, gpios);

#define USE_EXTERNAL_UFL_ANTENNA 0

int main(void)
{
    uint32_t boot_cycles = k_cycle_get_32();
    uint32_t boot_us = k_cyc_to_us_floor32(boot_cycles);

    if (gpio_is_ready_dt(&rf_en) && gpio_is_ready_dt(&rf_sel)) {
        gpio_pin_configure_dt(&rf_en, GPIO_OUTPUT_ACTIVE);
        gpio_pin_configure_dt(&rf_sel,
            USE_EXTERNAL_UFL_ANTENNA ? GPIO_OUTPUT_ACTIVE : GPIO_OUTPUT_INACTIVE);
    }

    if (!gpio_is_ready_dt(&led)) {
        return 0;
    }
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

    printk("\n========================================\n");
    printk("Target: Seeed Studio XIAO ESP32C6 (RISC-V)\n");
    printk("Kernel ticks to main(): %u us (%u cycles)\n", boot_us, boot_cycles);
    printk("Antenna Mode: %s\n", USE_EXTERNAL_UFL_ANTENNA ? "External U.FL" : "Internal Ceramic");
    printk("========================================\n");

    while (1) {
        gpio_pin_toggle_dt(&led);
        k_msleep(500);
    }
    return 0;
}
