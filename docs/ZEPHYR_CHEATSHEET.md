# Zephyr RTOS Quick Reference & Cheatsheet

A concise quick-reference card for everyday Zephyr RTOS development on the **Seeed Studio XIAO ESP32C6**.

---

## 🛠️ Essential West CLI Commands

| Command | Purpose |
| :--- | :--- |
| `west build -b xiao_esp32c6 <path> -p auto` | Incremental build for XIAO ESP32C6 |
| `west build -b xiao_esp32c6 <path> -p always` | Pristine rebuild (cleans cache completely) |
| `west flash` | Flash compiled firmware to target board |
| `west flash --esp-port COM3 --esp-baud-rate 921600` | Flash to specific COM port at high speed |
| `west build -t menuconfig` | Open interactive terminal Kconfig browser |
| `west build -t guiconfig` | Open graphical Tk-based Kconfig configurator |
| `west update` | Fetch/sync external git modules (hal_espressif, etc.) |
| `west topdir` | Display the root of the West workspace |

---

## 🌳 Devicetree C Macro Cheat Sheet

```c
/* 1. Get a node by alias */
#define MY_LED DT_ALIAS(led0)

/* 2. Get a node by label */
#define MY_I2C DT_NODELABEL(i2c0)

/* 3. Check if a node exists */
#if DT_NODE_EXISTS(DT_ALIAS(my_sensor))
    /* node is present */
#endif

/* 4. Extract property values */
int freq = DT_PROP(DT_NODELABEL(i2c0), clock_frequency);

/* 5. Driver Spec Helpers */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct i2c_dt_spec sensor = I2C_DT_SPEC_GET(DT_NODELABEL(sensor_node));
static const struct pwm_dt_spec pwm = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));
```

---

## ⚡ Kernel API Cheat Sheet

### Thread Sleeping & Uptime
```c
k_msleep(100);              /* Sleep thread for 100 milliseconds */
k_usleep(500);              /* Sleep thread for 500 microseconds */
k_sleep(K_SECONDS(2));      /* Sleep thread for 2 seconds */
k_sleep(K_FOREVER);         /* Put thread to sleep indefinitely */
uint32_t now = k_uptime_get_32(); /* System uptime in milliseconds */
```

### Static Thread Definition
```c
#define STACK_SIZE 1024
#define PRIORITY   7

void my_thread_fn(void *p1, void *p2, void *p3) {
    while (1) {
        k_msleep(1000);
    }
}

K_THREAD_DEFINE(my_tid, STACK_SIZE, my_thread_fn, NULL, NULL, NULL, PRIORITY, 0, 0);
```

### Semaphores (Event Signaling)
```c
K_SEM_DEFINE(my_sem, 0, 1); /* Initial count: 0, Limit: 1 */

/* Producer / ISR context */
k_sem_give(&my_sem);

/* Consumer / Thread context */
if (k_sem_take(&my_sem, K_MSEC(500)) == 0) {
    /* Event received! */
}
```

### Message Queues (FIFO Buffers)
```c
struct data_packet {
    uint32_t id;
    int16_t value;
};

K_MSGQ_DEFINE(my_queue, sizeof(struct data_packet), 16, 4);

/* Send */
struct data_packet pkt = { .id = 1, .value = 100 };
k_msgq_put(&my_queue, &pkt, K_NO_WAIT);

/* Receive */
struct data_packet rx_pkt;
if (k_msgq_get(&my_queue, &rx_pkt, K_FOREVER) == 0) {
    /* Process rx_pkt */
}
```

### System Workqueues (Deferring from ISR)
```c
void my_work_handler(struct k_work *work) {
    /* Runs in thread context */
}
K_WORK_DEFINE(my_work, my_work_handler);

/* Inside ISR or Timer */
k_work_submit(&my_work);
```
