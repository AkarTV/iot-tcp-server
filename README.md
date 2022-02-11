# iot-tcp-server
A simple smart house TCP-server using only standard C/C++ libraries. It creates a connection between client devices and IoT devices. The server allows these devices to exchange messages with each other.
## How to build
1. Clone this project to your local machine (or download ZIP)
2. Compile and link the .cpp files with any C++ compiler.

For example:
```
g++ event_selector.cpp server.cpp main.cpp -o iot-server
```
<hr>

## About some constants
Before using this program, it is recommended to chek some constants and change them if necessary.
1. **expected connections.** It means how many devices will be connected to the server. It is used in the vector::reserve function to avoid extra relocations, so it is not a critical constant.

In file event_selector.h:  
```c++
const int expected_connections = 15;
```
2. **buffer size.** Buffer size in bytes.

In file server.h:
```c++
const int buffer_size = 200;
```
3. **pass.** Password to connect the server.

In file server.h:
```c++
const char pass[] = "1234";
```
4. **port.** Port number (7000 by default)

In file main.cpp:
```c++
static int port = 7000;
```
<hr>

## How to use
After starting the program, the server begins to wait for a connection request on the listening socket. When connecting any device for the first time, it is necessary to enter the client name (for devices, it is necessary to enter "/d/" before the name). Example:
```
Enter your name (for devices enter '/d/' before name): /d/DoorSensor
```

If devices send something to the stream buffer, that message sends to all client devices. Example:
```
/d/DoorSensor: Opened
```

To send a message to some IoT device, the following construct is using: *<device_name>message*. Example:
```
</d/LightController>TurnOn
```
The client device can get the names of all IoT devices with the following command:
```
/a
```

Message processing is going on the side of clients and IoT devices.
