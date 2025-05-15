# ProxiGuard

**ProxiGuard** is a smart gate control system designed using an ESP32, ultrasonic sensor, servo motor, Bluetooth module, and LCD. It detects a vehicle approaching the gate and allows access through voice commands over Bluetooth.

##  Features

- Ultrasonic sensor detects car distance
- Automatic LED and buzzer alerts when a vehicle is detected
- Bluetooth voice command to unlock the gate
- Servo motor for gate control
- LCD displays system status and distance

##  How It Works

1. The ultrasonic sensor measures the distance between the gate and the car.
2. If the vehicle is within 70 cm:
   - LED turns ON
   - Buzzer alerts the guard
   - System waits for Bluetooth voice command **"Unlock"**
3. Once the command is received, the servo motor opens the gate.
4. If the car moves beyond 80 cm, the gate closes.

##  Components Used

- ESP32 microcontroller
- HC-SR04 Ultrasonic Sensor
- SG90 Servo Motor
- HC-05 Bluetooth Module
- LCD Display (I2C)
- Buzzer, LED, and Resistors
- Power Supply

##  Setup Instructions

1. Connect all components to the ESP32 as per the circuit diagram.
2. Upload the Arduino code to the ESP32.
3. Connect your mobile Bluetooth to the HC-05 module.
4. Send the voice command “Unlock” to open the gate.
