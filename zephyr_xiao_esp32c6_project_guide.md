# Project Context: Zephyr RTOS on Seeed Studio XIAO ESP32C6

## 1. System & Hardware Specification
- **IDE / Host OS:** Antigravity IDE on Windows 11 (PowerShell / WSL2 compatible)
- **Board:** Seeed Studio XIAO ESP32C6
- **SoC:** Espressif ESP32-C6 (Dual 32-bit RISC-V: 160 MHz High-Performance Core + 20 MHz Low-Power Core)
- **Memory:** 512 KB SRAM, 4 MB Embedded SPI Flash
- **Radios:** 2.4 GHz Wi-Fi 6 (802.11ax), BLE 5.3, IEEE 802.15.4 (Thread / Zigbee / Matter)
- **Zephyr HWMv2 Board Targets:**
  - High-Performance Core (Primary): `xiao_esp32c6/esp32c6/hpcore`
  - Low-Power Core (ULP Coprocessor): `xiao_esp32c6/esp32c6/lpcore`

---

## 2. Critical XIAO ESP32C6 Hardware Pinout & Quirks
1. **Onboard User LED (Yellow):** Connected to `GPIO15` (aliased as `led0` in Zephyr Devicetree).
2. **RF Antenna Switch (Ceramic vs. External U.FL):**
   - `GPIO3`: RF Switch Power Enable (**Active LOW** — must be driven `0` before or during Wi-Fi/BLE init, otherwise the RF switch is unpowered and signal attenuation exceeds 25 dB).
   - `GPIO14`: Antenna Select (`0` = Onboard Ceramic Antenna [Default], `1` = External U.FL Antenna).
3. **Native USB Serial/JTAG (`usb_serial`):**
   - Console and flashing run directly over the ESP32-C6 built-in USB Serial/JTAG controller (`GPIO12` D-, `GPIO13` D+).
   - **Watchdog Reset Required on Flash:** After `west flash`, the USB Serial/JTAG controller can hold the chip in download mode. Always append `--reset-type watchdog-reset` to automatically reboot into the application.
   - **Bootloader Recovery:** If a bad sleep/clock config drops the USB port, hold the tiny **BOOT** button, tap **RESET** (or plug in USB-C), then release **BOOT**.

---

## 3. Baseline Benchmark Reference (Zephyr v4.x vs. ESP-IDF v5.x)
| Metric | Zephyr RTOS (Tuned) | Zephyr RTOS (Default) | ESP-IDF (FreeRTOS) |
| :--- | :--- | :--- | :--- |
| **Minimal Flash** | ~24 KB | ~64 KB | ~185 KB |
| **Minimal Static RAM** | ~7 KB | ~12 KB | ~24 KB |
| **Wi-Fi + BLE Hub Flash** | ~940 KB | ~1,180 KB | ~960 KB |
| **Wi-Fi + BLE Hub SRAM** | ~128 KB | ~158 KB | ~126 KB |
| **Cold Boot Time** | ~21 ms | ~38 ms | ~72 ms (26 ms tuned) |
| **Standard ISR Latency** | ~1.9 $\mu\text{s}$ | ~2.1 $\mu\text{s}$ | ~1.2 $\mu\text{s}$ |
| **Zero-Latency IRAM ISR** | ~0.32 $\mu\text{s}$ (`IRQ_ZERO_LATENCY`) | N/A | ~0.28 $\mu\text{s}$ (Level 3+ IRAM) |

---

## 4. Environment & Binary Blob Initialization
Run once inside the Zephyr workspace (`zephyrproject`) terminal in Antigravity IDE:

```powershell
# Activate Zephyr virtual environment (Windows PowerShell)
.\.venv\Scripts\Activate.ps1

# Ensure Espressif Wi-Fi 6 / BLE / 802.15.4 PHY binary blobs are fetched
west blobs fetch hal_espressif
```

---

## 5. Starter Project Structure
Have the Antigravity IDE agent generate this directory tree supporting both bare-metal bring-up and the Wi-Fi 6 + MQTT telemetry node:

```text
xiao_c6_zephyr_app/
├── CMakeLists.txt
├── prj.conf
├── prj_minimal.conf
├── prj_wifi6.conf
├── boards/
│   └── xiao_esp32c6_esp32c6_hpcore.overlay
└── src/
    ├── main.c
    └── wifi_mqtt.c
```

### `CMakeLists.txt`
Automatically compiles `src/wifi_mqtt.c` when `CONFIG_WIFI` and `CONFIG_MQTT_LIB` are enabled via `prj_wifi6.conf`, or falls back to `src/main.c` for minimal/GPIO builds:

```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(xiao_c6_zephyr_bringup)

if(CONFIG_WIFI AND CONFIG_MQTT_LIB)
    target_sources(app PRIVATE src/wifi_mqtt.c)
else()
    target_sources(app PRIVATE src/main.c)
endif()
```

### `prj.conf` (Standard Bring-Up with USB Console + GPIO)
```ini
# Core & GPIO
CONFIG_GPIO=y
CONFIG_MAIN_STACK_SIZE=2048

# Console over built-in ESP32-C6 USB Serial/JTAG
CONFIG_SERIAL=y
CONFIG_CONSOLE=y
CONFIG_UART_CONSOLE=y

# High-resolution cycle counter for boot & latency benchmarking
CONFIG_TIMING_FUNCTIONS=y

# Enable Thread Info for Espressif OpenOCD debugging
CONFIG_DEBUG_THREAD_INFO=y
```

### `prj_minimal.conf` (Size-Optimized ~24 KB Flash Profile)
```ini
CONFIG_GPIO=y
CONFIG_SERIAL=n
CONFIG_CONSOLE=n
CONFIG_UART_CONSOLE=n
CONFIG_PRINTK=n
CONFIG_BOOT_BANNER=n
CONFIG_SIZE_OPTIMIZATIONS=y
CONFIG_ASSERT=n
CONFIG_ERRNO=n
CONFIG_MAIN_STACK_SIZE=1024
CONFIG_IDLE_STACK_SIZE=512
CONFIG_ISR_STACK_SIZE=1024
```

### `prj_wifi6.conf` (Wi-Fi 6 Station + DHCPv4 + DNS + MQTT Telemetry Profile)
```ini
# GPIO & Console over USB Serial/JTAG
CONFIG_GPIO=y
CONFIG_SERIAL=y
CONFIG_CONSOLE=y
CONFIG_UART_CONSOLE=y
CONFIG_PRINTK=y

# ESP32-C6 Wi-Fi 6 & L2 Ethernet Bridge
CONFIG_WIFI=y
CONFIG_WIFI_ESP32=y
CONFIG_NET_L2_ETHERNET=y
CONFIG_ESP32_WIFI_STA_AUTO_DHCPV4=y

# Core Zephyr Networking Stack
CONFIG_NETWORKING=y
CONFIG_NET_IPV4=y
CONFIG_NET_IPV6=n
CONFIG_NET_TCP=y
CONFIG_NET_UDP=y
CONFIG_NET_DHCPV4=y
CONFIG_DNS_RESOLVER=y

# Network Management Events & BSD Sockets
CONFIG_NET_MGMT=y
CONFIG_NET_MGMT_EVENT=y
CONFIG_NET_MGMT_EVENT_INFO=y
CONFIG_NET_MGMT_EVENT_STACK_SIZE=4096
CONFIG_NET_SOCKETS=y
CONFIG_POSIX_API=y

# MQTT Client Library
CONFIG_MQTT_LIB=y
CONFIG_MQTT_LIB_TLS=n

# Memory & Buffer Pool Tuning (Required for ESP32 Wi-Fi Blob + Zephyr net_pkt)
CONFIG_HEAP_MEM_POOL_SIZE=65536
CONFIG_MAIN_STACK_SIZE=4096
CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE=2048
CONFIG_NET_TX_STACK_SIZE=2048
CONFIG_NET_RX_STACK_SIZE=2048
CONFIG_NET_PKT_RX_COUNT=16
CONFIG_NET_PKT_TX_COUNT=16
CONFIG_NET_BUF_RX_COUNT=32
CONFIG_NET_BUF_TX_COUNT=32

# Interactive Wi-Fi Shell (Allows testing via serial terminal: `wifi connect ...`)
CONFIG_SHELL=y
CONFIG_NET_SHELL=y
CONFIG_WIFI_SHELL=y
```

### `boards/xiao_esp32c6_esp32c6_hpcore.overlay`
```dts
/ {
    aliases {
        led0 = &user_led;
    };

    leds {
        compatible = "gpio-leds";
        user_led: led_0 {
            gpios = <&gpio0 15 GPIO_ACTIVE_HIGH>;
            label = "User Yellow LED";
        };
    };

    rf_switch {
        compatible = "gpio-leds";
        rf_switch_en: rf_en {
            gpios = <&gpio0 3 GPIO_ACTIVE_LOW>;
            label = "RF Switch Power Enable";
        };
        rf_ant_sel: rf_sel {
            gpios = <&gpio0 14 GPIO_ACTIVE_HIGH>;
            label = "RF Antenna Select (0=Ceramic, 1=U.FL)";
        };
    };
};

&usb_serial {
    status = "okay";
};

&wifi {
    status = "okay";
};
```

---

## 6. Application Source Files

### `src/main.c` (Baseline Boot & GPIO Bring-Up)
```c
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
```

### `src/wifi_mqtt.c` (Wi-Fi 6 Station + MQTT Telemetry Node)
Configures the XIAO ESP32C6 RF switch, connects to your 2.4 GHz Wi-Fi 6 (802.11ax) or Wi-Fi 4 AP, waits for DHCPv4 lease assignment, and publishes JSON telemetry every 5 seconds:

```c
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_Event.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

/* ---------------- CONFIGURATION ---------------- */
#define WIFI_SSID                "YOUR_WIFI_SSID"
#define WIFI_PSK                 "YOUR_WIFI_PASSWORD"
#define USE_EXTERNAL_UFL_ANTENNA 0

/* Public test broker IP (test.mosquitto.org = 5.196.95.208) or local LAN broker */
#define MQTT_BROKER_ADDR         "5.196.95.208"
#define MQTT_BROKER_PORT         1883
#define MQTT_CLIENT_ID           "xiao_esp32c6_zephyr"
#define MQTT_PUB_TOPIC           "xiao_c6/telemetry"
#define MQTT_SUB_TOPIC           "xiao_c6/cmd"
/* ----------------------------------------------- */

#define LED0_NODE   DT_ALIAS(led0)
#define RF_EN_NODE  DT_NODELABEL(rf_switch_en)
#define RF_SEL_NODE DT_NODELABEL(rf_ant_sel)

static const struct gpio_dt_spec led    = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec rf_en  = GPIO_DT_SPEC_GET(RF_EN_NODE, gpios);
static const struct gpio_dt_spec rf_sel = GPIO_DT_SPEC_GET(RF_SEL_NODE, gpios);

static K_SEM_DEFINE(wifi_connected_sem, 0, 1);
static K_SEM_DEFINE(ipv4_obtained_sem, 0, 1);

static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;

/* MQTT client buffers & state */
static uint8_t rx_buffer[256];
static uint8_t tx_buffer[256];
static struct mqtt_client client_ctx;
static struct sockaddr_storage broker;
static struct zsock_pollfd fds[1];
static int nfds;
static bool mqtt_connected;

static void configure_xiao_rf_switch(void)
{
    if (gpio_is_ready_dt(&rf_en) && gpio_is_ready_dt(&rf_sel)) {
        /* GPIO3 Active-Low enables power to the onboard RF switch IC */
        gpio_pin_configure_dt(&rf_en, GPIO_OUTPUT_ACTIVE);
        /* GPIO14 selects between Ceramic (0) and External U.FL (1) */
        gpio_pin_configure_dt(&rf_sel,
            USE_EXTERNAL_UFL_ANTENNA ? GPIO_OUTPUT_ACTIVE : GPIO_OUTPUT_INACTIVE);
    }
}

static void wifi_event_handler(struct net_mgmt_event_callback *cb,
                               uint32_t mgmt_event, struct net_if *iface)
{
    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (mgmt_event == NET_EVENT_WIFI_CONNECT_RESULT) {
        if (status->status) {
            printk("[WIFI] Connection failed (%d)\n", status->status);
        } else {
            printk("[WIFI] Connected to AP (802.11ax/b/g/n)\n");
            k_sem_give(&wifi_connected_sem);
        }
    } else if (mgmt_event == NET_EVENT_WIFI_DISCONNECT_RESULT) {
        printk("[WIFI] Disconnected\n");
        mqtt_connected = false;
    }
}

static void ipv4_event_handler(struct net_mgmt_event_callback *cb,
                               uint32_t mgmt_event, struct net_if *iface)
{
    if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD) {
        char ip_buf[NET_IPV4_ADDR_LEN];
        struct net_if_ipv4 *ipv4 = iface->config.ip.ipv4;

        if (ipv4 && ipv4->unicast[0].ipv4.is_used) {
            net_addr_ntop(AF_INET, &ipv4->unicast[0].ipv4.address.in_addr,
                          ip_buf, sizeof(ip_buf));
            printk("[DHCP] IPv4 Address obtained: %s\n", ip_buf);
            k_sem_give(&ipv4_obtained_sem);
        }
    }
}

static int wifi_connect(void)
{
    struct net_if *iface = net_if_get_default();
    struct wifi_connect_req_params params = {0};

    params.ssid        = (const uint8_t *)WIFI_SSID;
    params.ssid_length = strlen(WIFI_SSID);
    params.psk         = (const uint8_t *)WIFI_PSK;
    params.psk_length  = strlen(WIFI_PSK);
    params.channel     = WIFI_CHANNEL_ANY;
    params.security    = WIFI_SECURITY_TYPE_PSK;
    params.band        = WIFI_FREQ_BAND_2_4_GHZ;
    params.mfp         = WIFI_MFP_OPTIONAL;

    printk("[WIFI] Connecting to SSID: %s ...\n", WIFI_SSID);
    return net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &params, sizeof(params));
}

static void mqtt_evt_handler(struct mqtt_client *const client,
                             const struct mqtt_evt *evt)
{
    switch (evt->type) {
    case MQTT_EVT_CONNACK:
        if (evt->result == 0) {
            mqtt_connected = true;
            printk("[MQTT] Connected to broker!\n");
        } else {
            printk("[MQTT] CONNACK failed: %d\n", evt->result);
        }
        break;
    case MQTT_EVT_DISCONNECT:
        mqtt_connected = false;
        printk("[MQTT] Disconnected (%d)\n", evt->result);
        break;
    case MQTT_EVT_PUBACK:
        printk("[MQTT] PUBACK packet id: %u\n", evt->param.puback.message_id);
        break;
    default:
        break;
    }
}

static int mqtt_connect_to_broker(void)
{
    struct sockaddr_in *broker4 = (struct sockaddr_in *)&broker;

    broker4->sin_family = AF_INET;
    broker4->sin_port   = htons(MQTT_BROKER_PORT);
    zsock_inet_pton(AF_INET, MQTT_BROKER_ADDR, &broker4->sin_addr);

    mqtt_client_init(&client_ctx);
    client_ctx.broker           = &broker;
    client_ctx.evt_cb           = mqtt_evt_handler;
    client_ctx.client_id.utf8   = (uint8_t *)MQTT_CLIENT_ID;
    client_ctx.client_id.size   = strlen(MQTT_CLIENT_ID);
    client_ctx.protocol_version = MQTT_VERSION_3_1_1;
    client_ctx.transport.type   = MQTT_TRANSPORT_NON_SECURE;

    client_ctx.rx_buf      = rx_buffer;
    client_ctx.rx_buf_size = sizeof(rx_buffer);
    client_ctx.tx_buf      = tx_buffer;
    client_ctx.tx_buf_size = sizeof(tx_buffer);

    int rc = mqtt_connect(&client_ctx);
    if (rc != 0) {
        printk("[MQTT] mqtt_connect failed: %d\n", rc);
        return rc;
    }

    fds[0].fd     = client_ctx.transport.tcp.sock;
    fds[0].events = ZSOCK_POLLIN;
    nfds          = 1;

    /* Wait for CONNACK */
    if (zsock_poll(fds, nfds, 5000) > 0) {
        mqtt_input(&client_ctx);
    }

    return mqtt_connected ? 0 : -EIO;
}

static int mqtt_publish_telemetry(uint32_t seq, uint32_t boot_us)
{
    char payload[128];
    struct mqtt_publish_param param = {0};

    snprintf(payload, sizeof(payload),
             "{\"board\":\"xiao_esp32c6\",\"seq\":%u,\"uptime_ms\":%lld,\"boot_us\":%u}",
             seq, k_uptime_get(), boot_us);

    param.message.topic.qos        = MQTT_QOS_0_AT_MOST_ONCE;
    param.message.topic.topic.utf8 = (uint8_t *)MQTT_PUB_TOPIC;
    param.message.topic.topic.size = strlen(MQTT_PUB_TOPIC);
    param.message.payload.data     = (uint8_t *)payload;
    param.message.payload.len      = strlen(payload);
    param.message_id               = seq;

    printk("[MQTT] Publishing -> %s: %s\n", MQTT_PUB_TOPIC, payload);
    return mqtt_publish(&client_ctx, &param);
}

int main(void)
{
    uint32_t boot_cycles = k_cycle_get_32();
    uint32_t boot_us     = k_cyc_to_us_floor32(boot_cycles);
    uint32_t seq         = 1;

    /* 1. Power on the XIAO ESP32C6 RF Antenna Multiplexer before Wi-Fi starts */
    configure_xiao_rf_switch();

    if (gpio_is_ready_dt(&led)) {
        gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    }

    printk("\n============================================\n");
    printk("XIAO ESP32C6 Wi-Fi 6 + MQTT Node (boot: %u us)\n", boot_us);
    printk("============================================\n");

    /* 2. Register Wi-Fi and IPv4 Event Callbacks */
    net_mgmt_init_event_callback(&wifi_cb, wifi_event_handler,
                                 NET_EVENT_WIFI_CONNECT_RESULT |
                                 NET_EVENT_WIFI_DISCONNECT_RESULT);
    net_mgmt_add_event_callback(&wifi_cb);

    net_mgmt_init_event_callback(&ipv4_cb, ipv4_event_handler,
                                 NET_EVENT_IPV4_ADDR_ADD);
    net_mgmt_add_event_callback(&ipv4_cb);

    /* Allow ESP32-C6 Wi-Fi MAC blob initialization to settle */
    k_sleep(K_SECONDS(1));

    /* 3. Connect to Wi-Fi AP & Wait for DHCPv4 */
    wifi_connect();
    k_sem_take(&wifi_connected_sem, K_FOREVER);
    k_sem_take(&ipv4_obtained_sem, K_FOREVER);

    /* 4. Connect to MQTT Broker & Publish Loop */
    while (1) {
        if (!mqtt_connected) {
            printk("[MQTT] Connecting to %s:%d ...\n", MQTT_BROKER_ADDR, MQTT_BROKER_PORT);
            if (mqtt_connect_to_broker() != 0) {
                k_sleep(K_SECONDS(5));
                continue;
            }
        }

        if (zsock_poll(fds, nfds, 1000) > 0) {
            mqtt_input(&client_ctx);
        }

        mqtt_live(&client_ctx);
        mqtt_publish_telemetry(seq++, boot_us);
        gpio_pin_toggle_dt(&led);

        k_sleep(K_SECONDS(5));
    }

    return 0;
}
```

---

## 7. Build, Flash, Footprint Report & Monitor Commands

### Option A: Build Standard GPIO / Boot Benchmark (`prj.conf`)
```powershell
west build -p always -b xiao_esp32c6/esp32c6/hpcore --sysbuild ./xiao_c6_zephyr_app
```

### Option B: Build Minimal ~24 KB Footprint Profile (`prj_minimal.conf`)
```powershell
west build -p always -b xiao_esp32c6/esp32c6/hpcore --sysbuild ./xiao_c6_zephyr_app -- -DCONF_FILE=prj_minimal.conf
```

### Option C: Build Wi-Fi 6 + MQTT Telemetry Profile (`prj_wifi6.conf`)
```powershell
west build -p always -b xiao_esp32c6/esp32c6/hpcore --sysbuild ./xiao_c6_zephyr_app -- -DCONF_FILE=prj_wifi6.conf
```

### Flash & Auto-Reboot via Watchdog Reset
```powershell
west flash --reset-type watchdog-reset
```

### Fast Incremental Flash (Only reflashes changed sectors)
```powershell
west flash --esp-diff --reset-type watchdog-reset
```

### Serial Monitor (Also supports interactive `wifi` shell commands)
```powershell
west espressif monitor
```

### Inspect RAM & Flash Footprint Breakdown
```powershell
west build -t rom_report
west build -t ram_report
```

---

## 8. Next Tasks for Antigravity IDE Agent
- [ ] **Task 1:** Scaffold `xiao_c6_zephyr_app/` using the files in Sections 5 & 6 and run `west build` with `prj.conf`, `prj_minimal.conf`, and `prj_wifi6.conf` to compare `rom_report` and `ram_report` outputs.
- [ ] **Task 2:** Update `WIFI_SSID`, `WIFI_PSK`, and `MQTT_BROKER_ADDR` in `src/wifi_mqtt.c`, flash using `west flash --reset-type watchdog-reset`, and verify JSON telemetry packets on `xiao_c6/telemetry`.
- [ ] **Task 3:** Add a zero-latency hardware timer or GPIO interrupt benchmark using `IRQ_CONNECT(..., IRQ_ZERO_LATENCY)` to measure RISC-V cycle latency while Wi-Fi 6 traffic is active.