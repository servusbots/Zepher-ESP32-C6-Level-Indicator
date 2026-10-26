/*
 * Lab 07: I2C Bus & Peripheral Scanner
 * Board: Seeed Studio XIAO ESP32C6
 *
 * Description:
 * Demonstrates the Zephyr I2C subsystem:
 * 1. Retrieves the primary I2C bus device (&i2c0).
 *    Pins on XIAO ESP32C6:
 *      D4 = GPIO 22 (SDA)
 *      D5 = GPIO 23 (SCL)
 * 2. Checks i2c_is_ready_dt().
 * 3. Scans all 7-bit addresses (0x01 to 0x7F) and prints a Linux-style
 *    'i2cdetect' matrix showing all discovered sensors/devices!
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lab07_i2c, LOG_LEVEL_INF);

/*
 * Use i2c0 defined on XIAO ESP32C6.
 * D4 = SDA (GPIO 22), D5 = SCL (GPIO 23)
 */
static const struct device *const i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));

static void scan_i2c_bus(const struct device *dev)
{
	uint8_t count = 0;
	uint8_t dummy = 0;

	printk("\n     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");

	for (uint8_t i = 0; i <= 0x7F; i += 16) {
		printk("%02x: ", i);
		for (uint8_t j = 0; j < 16; j++) {
			uint8_t addr = i + j;

			/* Skip reserved addresses (0x00, and 0x78-0x7F) */
			if (addr < 0x03 || addr > 0x77) {
				printk("   ");
				continue;
			}

			/*
			 * Attempt a 1-byte read transaction with 20ms timeout.
			 * If ACKed, a device responded!
			 */
			struct i2c_msg msgs[1];
			msgs[0].buf = &dummy;
			msgs[0].len = 1;
			msgs[0].flags = I2C_MSG_READ | I2C_MSG_STOP;

			int ret = i2c_transfer(dev, msgs, 1, addr);
			if (ret == 0) {
				printk("%02x ", addr);
				count++;
			} else {
				printk("-- ");
			}
		}
		printk("\n");
	}

	printk("\n>>> Discovered %u I2C device(s) on bus %s.\n\n", count, dev->name);
}

int main(void)
{
	LOG_INF("==========================================================");
	LOG_INF("  Seeed Studio XIAO ESP32C6 - Lab 07 I2C Bus Scanner     ");
	LOG_INF("==========================================================");
	LOG_INF("  I2C Bus: %s", i2c_dev->name);
	LOG_INF("  SDA Pin: D4 (GPIO 22)");
	LOG_INF("  SCL Pin: D5 (GPIO 23)");

	if (!device_is_ready(i2c_dev)) {
		LOG_ERR("Error: I2C controller %s is not ready!", i2c_dev->name);
		return 0;
	}

	LOG_INF("Starting continuous I2C scans (repeats every 5 seconds)...");
	LOG_INF("Connect any I2C sensor (OLED, BME280, MPU6050, etc.) to D4/D5.");

	while (1) {
		scan_i2c_bus(i2c_dev);
		k_msleep(5000);
	}

	return 0;
}
