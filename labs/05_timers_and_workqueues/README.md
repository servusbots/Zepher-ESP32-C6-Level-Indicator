# Lab 05: Kernel Timers & System Workqueues

In embedded systems, you will often need to respond to hardware interrupts or timer events that require significant processing time (such as calculating cryptographic hashes, reading slow sensors over I2C, or writing to flash).

Doing heavy work in an ISR is a cardinal sin: it blocks all lower-priority interrupts and can trigger kernel watchdog resets.

Zephyr's **Workqueue Subsystem** provides the standard solution: **Bottom-Half Deferral**.

---

## 🎯 Objectives
1. Configure and start a periodic software timer using `k_timer_start()`.
2. Understand the differences between **ISR Context** and **Thread Context**.
3. Defer work from an ISR to the kernel **System Workqueue** using `k_work_submit()`.

---

## ⏱️ Execution Flow
```text
  [Hardware Timer / ISR]
            |
            | (Fast: < 5 microseconds)
            v
     timer_expiry_handler()
            |
            +--> k_work_submit(&sample_work_item)
            |
      (Exits ISR immediately)
            :
            v
  [Kernel Scheduler]
            v
  [System Workqueue Thread]
            |
            +--> work_handler()
                 (Thread Context: Safe to sleep, do I/O, take mutexes)
```

---

## 🚀 Building & Flashing

```powershell
. .\scripts\env.ps1
west build -b xiao_esp32c6 labs\05_timers_and_workqueues -p auto
west flash
.\scripts\monitor.ps1
```
