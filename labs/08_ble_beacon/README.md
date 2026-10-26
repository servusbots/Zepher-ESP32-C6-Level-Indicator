# Lab 08: Bluetooth Low Energy (BLE 5.3) Beacon

The Espressif ESP32-C6 includes a world-class 2.4 GHz radio supporting Bluetooth 5.3 Low Energy. Zephyr's Bluetooth stack is fully qualified and supports central, peripheral, broadcaster, and observer roles.

---

## 🎯 Objectives
1. Configure and enable the Zephyr Bluetooth LE stack (`bt_enable`).
2. Construct GAP advertising packets with flags, device name, and custom manufacturer data (`struct bt_data`).
3. Start non-connectable advertising (`bt_le_adv_start`).
4. Dynamically update live sensor payload telemetry (`bt_le_adv_update_data`).

---

## 📱 Testing with Your Phone
1. Install the free **nRF Connect for Mobile** app (available on Google Play Store and Apple App Store).
2. Open the app and tap **Scan**.
3. Look for the advertisement broadcasting as **`XIAO-ESP32C6-Zephyr`**.
4. Tap on the device details to inspect the live updating **Manufacturer Data** packet containing the broadcast counter and simulated temperature!

---

## 🚀 Building & Flashing

```powershell
. .\scripts\env.ps1
west build -b xiao_esp32c6 labs\08_ble_beacon -p auto
west flash
.\scripts\monitor.ps1
```
