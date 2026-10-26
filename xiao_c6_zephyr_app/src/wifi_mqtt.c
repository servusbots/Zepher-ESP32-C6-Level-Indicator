#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_event.h>
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
