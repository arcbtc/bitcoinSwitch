
<center>
![image](https://user-images.githubusercontent.com/33088785/166831893-a112965e-eb05-4e46-add6-6191c6b3f4fc.png)
![boards](https://user-images.githubusercontent.com/33088785/166829474-a28ca2b7-dd3e-46d4-89d3-8a10bf1d3fad.png)
</center>

# bitcoinSwitch💡
Turn things on with bitcoin.

Compatible with any ESP32 microcontroller, with or without a screen, supporting native LN invoices and static LNURLPay links.

### Includes portal for easy setup
![portal](https://user-images.githubusercontent.com/33088785/166824760-bc612411-7663-4a78-9331-61390042e3ae.gif)

### Install instructions
- Install <a href="https://www.arduino.cc/en/software">Arduino IDE 1.8.19</a>
- Install ESP32 boards, using <a href="https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html#installing-using-boards-manager">boards manager</a>
- Role back ESP32 boards to 2.0.1 in boards manager
![image](https://user-images.githubusercontent.com/33088785/161862832-1269a12e-16ce-427c-9a92-df3ee573a1fb.png)

- Download this repo
- Copy these <a href="libraries">libraries</a> into your Arduino install "libraries" folder
- Open this <a href="bitcoinSwitch.ino">bitcoinSwitch.ino</a> file in the Arduino IDE
- Select the correct ESP32 board from tools>board
- Upload to device

> On M5Stack press A to launch portal within first few secs of startup, for all other ESP32 devices specify a GPIO to detect capacitance change, and touch within first few secs of startup.
> Default password is "ToTheMoon"


![trigger](https://user-images.githubusercontent.com/33088785/166829947-d0194b32-19fc-4a16-83d3-dc6f9af9337c.gif)
