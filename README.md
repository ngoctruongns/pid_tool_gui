# PID Tuning GUI (Qt5 C++)

Simple GUI tool to tune PID for motor control via Arduino over Serial.

Build (Linux) prerequisites:
- Qt5 development packages (Qt5 Widgets, Qt5 SerialPort, Qt5 Charts)
- CMake >= 3.5

Example (Ubuntu):

```bash
sudo apt install qtbase5-dev libqt5serialport5-dev libqt5charts5-dev cmake build-essential
mkdir build && cd build
cmake ..
make -j
./pid_tool_gui
```

Notes:
- The tool lists available serial ports. Use `Find Port` then `Open`.
- Incoming text lines that are CSV like `t,val1,val2,val3` will be plotted (up to 3 series).
- PID update button sends `PID Kp Ki Kd\n` to the Arduino; adapt Arduino sketch accordingly.
