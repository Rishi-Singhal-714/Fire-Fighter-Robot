# 🔥 Autonomous Firefighting Robot 🚒

An intelligent ESP32-based robot designed to detect and extinguish fires in indoor environments. It autonomously navigates around obstacles within a 30cm range using IR and ultrasonic sensors and extinguishes fire using a mini fan-based suppression system.

---

## 📌 Features

- 🔥 **Fire Detection** using IR flame sensors  
- 🚧 **Obstacle Avoidance** using ultrasonic sensors (up to 30cm)  
- 🤖 **Autonomous Navigation** with real-time sensor feedback  
- 💨 **Fire Extinguishing** using DC fan actuator  
- 📡 **Wi-Fi Remote Control** for manual operation via web or mobile interface  
- 🔋 Low-power design based on ESP32 microcontroller  
- 🧠 Expandable with computer vision (optional future upgrade)  

---

## 📸 Project Preview
![1746870092782](https://github.com/user-attachments/assets/290dda99-44ea-4614-a18a-f659ab7211eb)
![1746870094081](https://github.com/user-attachments/assets/c8e32c65-f3c4-4cdc-a755-4fef58940915)
*A look at the robot in action (include a real or simulated image)*

---

## 🛠️ Tech Stack

- **Microcontroller**: ESP32  
- **Sensors**: IR Flame Sensor, Ultrasonic Sensor (HC-SR04)  
- **Actuators**: 4x DC Motors (for movement), 1x DC Pump (for extinguishing)  
- **Programming Language**: Embedded C / Arduino C  
- **Development Tools**: Arduino IDE, Fritzing (for circuit design)  

---

## 🧩 Hardware Components

| Component             | Quantity |
|-----------------------|----------|
| ESP32 Dev Board       | 1        |
| IR Flame Sensor       | 3        |
| Ultrasonic Sensor     | 1        |
| DC Motors + Wheels    | 4        |
| DC Pump               | 1        |
| Motor Driver (L298N)  | 1        |
| Battery Pack 9V       | 1        |
| Batter Cell 3.3V      | 2        |
| Jumper Wires          | -        |

## 🔁 Working Principle

1. **Start-Up**: On boot, the ESP32 initializes all connected sensors and actuators.
2. **Detection**: Continuously monitors the environment for flame signals using IR sensors and detects nearby obstacles with ultrasonic sensing (up to 30cm).
3. **Navigation**: Automatically avoids obstacles and navigates toward detected flames.
4. **Extinguish**: Once within effective range, the robot activates a DC fan to extinguish the fire.
5. **Wi-Fi Remote Control (Optional Mode)**: The robot can switch to manual control via a web interface or mobile app over Wi-Fi, allowing real-time control and monitoring.

