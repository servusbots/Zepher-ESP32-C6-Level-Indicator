/*
 * Seeed Studio XIAO ESP32C6 - Bulls-Eye Level Indicator with MPU-6050 & SSD1306
 *
 * Requirements:
 * 1. Power-on 5-second automatic zero-offset calibration with on-screen progress.
 * 2. Real-time Bulls-Eye Level Indicator target with concentric rings & crosshairs.
 * 3. Dynamic bubble indicator mapped to calibrated Pitch & Roll angles.
 * 4. Strobe / screen flash when the device is perfectly LEVEL (< 1.0 degree).
 * 5. Telemetry output over native USB-Serial-JTAG (COM port).
 *
 * Hardware Wiring (I2C0):
 *   VCC -> 3.3V
 *   GND -> GND
 *   SDA -> XIAO D4 (GPIO 22)
 *   SCL -> XIAO D5 (GPIO 23)
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/display.h>
#include <zephyr/sys/printk.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "font5x7.h"

#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define FB_SIZE        ((SCREEN_WIDTH * SCREEN_HEIGHT) / 8)

#define LEVEL_THRESHOLD_DEG  1.0f
#define CALIBRATION_TIME_MS  5000

/* Display framebuffer in SSD1306 vertical page format */
static uint8_t fb[FB_SIZE];

/* Devices */
static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static const struct device *mpu_dev = DEVICE_DT_GET(DT_NODELABEL(mpu6050));
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

/* Calibration offsets */
static double bias_accel_x = 0.0;
static double bias_accel_y = 0.0;

/* --- Framebuffer Graphics Primitives --- */

static void fb_clear(void)
{
    memset(fb, 0, sizeof(fb));
}

static void fb_invert(void)
{
    for (size_t i = 0; i < sizeof(fb); i++) {
        fb[i] = ~fb[i];
    }
}

static void fb_draw_pixel(int x, int y, uint8_t color)
{
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        if (color) {
            fb[x + (y / 8) * SCREEN_WIDTH] |= (1 << (y % 8));
        } else {
            fb[x + (y / 8) * SCREEN_WIDTH] &= ~(1 << (y % 8));
        }
    }
}

static void fb_draw_line(int x0, int y0, int x1, int y1, uint8_t color)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        fb_draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void fb_draw_circle(int xc, int yc, int r, uint8_t color)
{
    int x = 0, y = r;
    int d = 3 - 2 * r;

    while (y >= x) {
        fb_draw_pixel(xc + x, yc + y, color);
        fb_draw_pixel(xc - x, yc + y, color);
        fb_draw_pixel(xc + x, yc - y, color);
        fb_draw_pixel(xc - x, yc - y, color);
        fb_draw_pixel(xc + y, yc + x, color);
        fb_draw_pixel(xc - y, yc + x, color);
        fb_draw_pixel(xc + y, yc - x, color);
        fb_draw_pixel(xc - y, yc - x, color);

        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

static void fb_fill_circle(int xc, int yc, int r, uint8_t color)
{
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r) {
                fb_draw_pixel(xc + x, yc + y, color);
            }
        }
    }
}

static void fb_draw_rect(int x, int y, int w, int h, uint8_t color)
{
    fb_draw_line(x, y, x + w - 1, y, color);
    fb_draw_line(x, y + h - 1, x + w - 1, y + h - 1, color);
    fb_draw_line(x, y, x, y + h - 1, color);
    fb_draw_line(x + w - 1, y, x + w - 1, y + h - 1, color);
}

static void fb_fill_rect(int x, int y, int w, int h, uint8_t color)
{
    for (int i = 0; i < h; i++) {
        fb_draw_line(x, y + i, x + w - 1, y + i, color);
    }
}

static void fb_draw_char(int x, int y, char c, uint8_t color)
{
    const uint8_t *glyph = font5x7_get_char(c);
    for (int col = 0; col < FONT5X7_WIDTH; col++) {
        uint8_t line = glyph[col];
        for (int row = 0; row < FONT5X7_HEIGHT; row++) {
            if (line & (1 << row)) {
                fb_draw_pixel(x + col, y + row, color);
            }
        }
    }
}

static void fb_draw_string(int x, int y, const char *str, uint8_t color)
{
    while (*str) {
        fb_draw_char(x, y, *str, color);
        x += FONT5X7_WIDTH + 1;
        str++;
    }
}

static void fb_flush(const struct device *dev)
{
    struct display_buffer_descriptor desc = {
        .buf_size = sizeof(fb),
        .width = SCREEN_WIDTH,
        .height = SCREEN_HEIGHT,
        .pitch = SCREEN_WIDTH,
    };
    display_write(dev, 0, 0, &desc, fb);
}

/* --- Sensor Reading & Calibration --- */

static int read_mpu6050_raw(double *ax, double *ay, double *az)
{
    struct sensor_value accel[3];
    int rc = sensor_sample_fetch(mpu_dev);
    if (rc != 0) {
        return rc;
    }

    rc = sensor_channel_get(mpu_dev, SENSOR_CHAN_ACCEL_XYZ, accel);
    if (rc != 0) {
        return rc;
    }

    *ax = sensor_value_to_double(&accel[0]);
    *ay = sensor_value_to_double(&accel[1]);
    *az = sensor_value_to_double(&accel[2]);
    return 0;
}

static void run_5sec_calibration(void)
{
    printk(">>> Starting 5-Second IMU Calibration...\n");
    int64_t start_time = k_uptime_get();
    int samples = 0;
    double sum_ax = 0.0, sum_ay = 0.0;

    while (1) {
        int64_t elapsed = k_uptime_get() - start_time;
        if (elapsed >= CALIBRATION_TIME_MS) {
            break;
        }

        double ax = 0.0, ay = 0.0, az = 0.0;
        if (read_mpu6050_raw(&ax, &ay, &az) == 0) {
            sum_ax += ax;
            sum_ay += ay;
            samples++;
        }

        /* Render on-screen calibration progress */
        fb_clear();
        fb_draw_string(20, 8, "CALIBRATING IMU", 1);
        fb_draw_string(14, 20, "Keep Device Level", 1);

        /* Progress Bar (outline + fill) */
        int bar_w = 90;
        int bar_h = 8;
        int bar_x = 19;
        int bar_y = 34;
        fb_draw_rect(bar_x, bar_y, bar_w, bar_h, 1);
        int fill_w = (int)((elapsed * (bar_w - 4)) / CALIBRATION_TIME_MS);
        if (fill_w > 0) {
            fb_fill_rect(bar_x + 2, bar_y + 2, fill_w, bar_h - 4, 1);
        }

        /* Countdown display */
        char countdown_str[32];
        double rem_sec = (CALIBRATION_TIME_MS - elapsed) / 1000.0;
        snprintf(countdown_str, sizeof(countdown_str), "Ready in: %.1fs", rem_sec);
        fb_draw_string(22, 48, countdown_str, 1);

        fb_flush(display_dev);
        k_msleep(50);
    }

    if (samples > 0) {
        bias_accel_x = sum_ax / samples;
        bias_accel_y = sum_ay / samples;
    }

    printk(">>> Calibration Complete (%d samples). Bias X: %.4f, Bias Y: %.4f\n",
           samples, bias_accel_x, bias_accel_y);

    /* Brief completion screen */
    fb_clear();
    fb_draw_string(16, 20, "CALIBRATION DONE!", 1);
    fb_draw_string(22, 36, "Starting Level...", 1);
    fb_flush(display_dev);
    k_msleep(750);
}

/* --- Main Application --- */

int main(void)
{
    printk("\n==================================================\n");
    printk("  Seeed XIAO ESP32C6 Bulls-Eye Level Indicator   \n");
    printk("==================================================\n");

    /* Initialize LED */
    if (gpio_is_ready_dt(&led)) {
        gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    }

    /* Initialize Display */
    if (!device_is_ready(display_dev)) {
        printk("FATAL: Display device not ready!\n");
        return 0;
    }
    display_blanking_off(display_dev);
    fb_clear();
    fb_draw_string(24, 26, "XIAO ESP32-C6", 1);
    fb_draw_string(18, 38, "LEVEL INDICATOR", 1);
    fb_flush(display_dev);
    k_msleep(1000);

    /* Initialize MPU-6050 */
    if (!device_is_ready(mpu_dev)) {
        printk("FATAL: MPU6050 sensor device not ready!\n");
        fb_clear();
        fb_draw_string(14, 24, "ERROR: MPU6050", 1);
        fb_draw_string(14, 36, "Check I2C Wiring", 1);
        fb_flush(display_dev);
        return 0;
    }
    printk("MPU-6050 sensor ready at I2C address 0x68.\n");

    /* Run 5-second automatic zero calibration */
    run_5sec_calibration();

    /* Filtered angle variables */
    double smooth_roll = 0.0;
    double smooth_pitch = 0.0;
    const double alpha = 0.30; /* Smoothing filter weight */

    bool flash_state = false;
    int64_t last_flash_toggle = 0;
    int64_t last_log_time = 0;

    const int target_cx = 64;
    const int target_cy = 32;

    while (1) {
        double raw_ax = 0.0, raw_ay = 0.0, raw_az = 0.0;
        if (read_mpu6050_raw(&raw_ax, &raw_ay, &raw_az) == 0) {
            /* Zero-offset calibration adjustment */
            double ax = raw_ax - bias_accel_x;
            double ay = raw_ay - bias_accel_y;
            double az = raw_az;

            /* Compute Pitch and Roll angles in degrees */
            double roll_rad = atan2(ay, az);
            double pitch_rad = atan2(-ax, sqrt(ay * ay + az * az));

            double roll_deg = roll_rad * (180.0 / 3.141592653589793);
            double pitch_deg = pitch_rad * (180.0 / 3.141592653589793);

            /* Exponential smoothing */
            smooth_roll = alpha * roll_deg + (1.0 - alpha) * smooth_roll;
            smooth_pitch = alpha * pitch_deg + (1.0 - alpha) * smooth_pitch;
        }

        /* Check level condition (< 1.0 degree in both axes) */
        bool is_level = (fabs(smooth_roll) < LEVEL_THRESHOLD_DEG) &&
                        (fabs(smooth_pitch) < LEVEL_THRESHOLD_DEG);

        fb_clear();

        /* 1. Draw Bulls-Eye Target Concentric Rings */
        fb_draw_circle(target_cx, target_cy, 24, 1); /* Outer ring (~10 deg) */
        fb_draw_circle(target_cx, target_cy, 14, 1); /* Middle ring (~5 deg) */
        fb_draw_circle(target_cx, target_cy, 5, 1);  /* Bullseye center ring (<1 deg) */

        /* 2. Draw Crosshairs */
        fb_draw_line(target_cx - 28, target_cy, target_cx - 7, target_cy, 1);
        fb_draw_line(target_cx + 7, target_cy, target_cx + 28, target_cy, 1);
        fb_draw_line(target_cx, target_cy - 28, target_cx, target_cy - 7, 1);
        fb_draw_line(target_cx, target_cy + 7, target_cx, target_cy + 28, 1);

        /* 3. Compute dynamic bubble position (2.2 pixels per degree) */
        double px_per_deg = 2.2;
        int bubble_x = target_cx + (int)round(smooth_roll * px_per_deg);
        int bubble_y = target_cy - (int)round(smooth_pitch * px_per_deg);

        /* Clamp bubble inside display bounds */
        if (bubble_x < 6) bubble_x = 6;
        if (bubble_x > 121) bubble_x = 121;
        if (bubble_y < 6) bubble_y = 6;
        if (bubble_y > 57) bubble_y = 57;

        /* Draw filled bubble */
        fb_fill_circle(bubble_x, bubble_y, 3, 1);

        /* 4. Draw Angle Text Readouts */
        char roll_buf[16];
        char pitch_buf[16];
        snprintf(roll_buf, sizeof(roll_buf), "R:%+.1f~", smooth_roll);
        snprintf(pitch_buf, sizeof(pitch_buf), "P:%+.1f~", smooth_pitch);

        fb_draw_string(2, 2, roll_buf, 1);
        fb_draw_string(2, 54, pitch_buf, 1);

        /* 5. Level Status & Screen Flash Effect */
        int64_t now = k_uptime_get();
        if (is_level) {
            /* Level message */
            fb_draw_string(76, 54, "*LEVEL*", 1);

            /* Strobe/Flash display every 150 ms */
            if (now - last_flash_toggle >= 150) {
                flash_state = !flash_state;
                last_flash_toggle = now;
            }

            if (flash_state) {
                fb_invert(); /* Invert screen colors for prominent visual flash */
                if (led.port) {
                    gpio_pin_set_dt(&led, 1); /* Active-Low LED ON */
                }
            } else {
                if (led.port) {
                    gpio_pin_set_dt(&led, 0); /* Active-Low LED OFF */
                }
            }
        } else {
            /* Not level - regular display, LED off */
            flash_state = false;
            if (led.port) {
                gpio_pin_set_dt(&led, 0);
            }
        }

        /* 6. Push buffer to OLED */
        fb_flush(display_dev);

        /* 7. Periodic Telemetry Log over USB Console (every 500 ms) */
        if (now - last_log_time >= 500) {
            last_log_time = now;
            printk("[%s] Roll: %+.2f deg | Pitch: %+.2f deg | Bubble: (%d, %d)\n",
                   is_level ? "LEVEL" : "TILT ",
                   smooth_roll, smooth_pitch, bubble_x, bubble_y);
        }

        k_msleep(40); /* ~25 FPS update rate */
    }

    return 0;
}
