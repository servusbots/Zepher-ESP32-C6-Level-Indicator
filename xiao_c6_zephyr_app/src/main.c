#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
    uint32_t boot_cycles = k_cycle_get_32();
    uint32_t boot_us = k_cyc_to_us_floor32(boot_cycles);

    if (!gpio_is_ready_dt(&led)) {
        printk("Error: User LED (led0) not ready!\n");
        return 0;
    }
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

    printk("\n========================================\n");
    printk("  Seeed Studio XIAO ESP32C6 - Zephyr RTOS\n");
    printk("  Board: xiao_esp32c6 (RISC-V HP Core)   \n");
    printk("========================================\n");
    printk("Kernel ticks to main(): %u us (%u cycles)\n", boot_us, boot_cycles);
#if defined(CONFIG_XIAO_ESP32C6_EXT_ANTENNA)
    printk("Antenna Mode: External U.FL\n");
#else
    printk("Antenna Mode: Internal Ceramic\n");
#endif
    printk("Yellow User LED (GPIO 15) Active-LOW blinking...\n");
    printk("========================================\n\n");

    uint32_t count = 0;
    while (1) {
        gpio_pin_toggle_dt(&led);
        count++;
        if (count % 10 == 0) {
            printk("[Uptime: %lld ms] Heartbeat toggle #%u (LED active-low)\n",
                   k_uptime_get(), count);
        }
        k_msleep(500);
    }
    return 0;
}
