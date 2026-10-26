# Lab 04: RTOS Multithreading & Synchronization

In bare-metal embedded programming, handling multiple asynchronous tasks requires complex state machines inside a `while(1)` super-loop. Zephyr RTOS provides true preemptive multithreading with thread priorities, message queues, and semaphores.

---

## 🎯 Objectives
1. Understand Zephyr's static thread definition (`K_THREAD_DEFINE`).
2. Learn how **Thread Priorities** work (Lower number = Higher priority).
3. Safely pass data between threads using a **Message Queue** (`k_msgq_put`, `k_msgq_get`).
4. Signal events between threads using a **Binary Semaphore** (`k_sem_give`, `k_sem_take`).

---

## 🏗️ Architecture Diagram
```text
 +------------------+                      +------------------+
 |  PRODUCER THREAD |                      |  CONSUMER THREAD |
 |  (Priority 7)    |                      |  (Priority 6)    |
 +--------+---------+                      +--------+---------+
          |                                         |
          |--- [k_msgq_put] ---> SENSOR QUEUE ----->| [k_msgq_get]
          |                      (Ring Buffer)      | (Pulsing LED)
          |                                         |
          |--- [k_sem_give] ---> ALERT SEMAPHORE -->| [k_sem_take]
                                 (Binary Flag)      | (Rapid Blink)
```

---

## 🚀 Building & Flashing

```powershell
. .\scripts\env.ps1
west build -b xiao_esp32c6 labs\04_rtos_multithreading -p auto
west flash
.\scripts\monitor.ps1
```

---

## 🧠 Key Zephyr Concepts Learned
* `K_THREAD_DEFINE(name, stack_size, entry_fn, p1, p2, p3, prio, options, delay)`: Allocates stack, initializes thread control block, and registers it with the kernel scheduler at compile time.
* `K_MSGQ_DEFINE(name, msg_size, max_msgs, align)`: Creates a thread-safe ring buffer.
* `K_SEM_DEFINE(name, initial_count, count_limit)`: Creates a counting or binary semaphore.
