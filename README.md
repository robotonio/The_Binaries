# The_Binaries
This is the research project of "The Binaries" team, in the FLL "INTO ORBIT" competition, in 2019. The project's object is the exploitation of "Soft Robotics" technology in space and for this purpose, the team has developed a system which compares the pros and cons of soft &amp; hard grippers.

The name of the S/H system comes from the words Soft vs. Hard (actuators).
The main purpose of the system is to show some of the advantages of Soft Robotics in the field of actuators. The above objective should not be construed as an attempt to prove that soft actuators are "better" than traditional actuators (hard), as Soft Robotics can not substitute current technology but can help address specific problems. The system is controlled by a central console, as well as remote control (Bluetooth) via an Android application developed for that purpose.

![alt text](https://github.com/robotonio/The_Binaries/blob/master/console_description.png)

The system is activated with the red button (B.1). When in operation, the red LED (L.1) is on, otherwise off. When the system is turned on, the THE BINARIES message appears on the LCD for 3 seconds, then the default function, which is "MANUAL," appears on the first line of the screen. The operation of the system can be changed to "AUTO" by pressing the black button (B.2) and returns to "MANUAL" in the same way.

• "MANUAL" mode: the actuators (soft & hard) open and close by pressing the green buttons B.3 & B.4 respectively. When the system is in "MANUAL" mode, the first green LED (L.2) is lit.

• "AUTO" mode: each actuator automatically closes when the ultrasonic sensor detects an object. The actuator remains closed until button B.3 is pressed. When the system is in "AUTO" mode, the second green LED (L.3) is lit.

There are also three manual air switches in the console. The first switch (S.1) should always be open to allow the Soft Actuator to bleed out. The second and third allow the air supply to the Soft Actuator to which they are connected. If the 1st actuator is used, the first switch (S.2) must be open and the 2nd switch (S.3) closed. The S.2 & S.3 switches must be inverted to activate the 2nd soft actuator.

There is also the third green LED (L.4) on the console that lights when a Bluetooth device is connected to the system. Finally, under the LCD, there is a potentiometer (P.1) used to adjust the air pressure that closes the soft actuator. The procedure for setting the pressure is as follows: the user presses B.3 for 5 seconds until "CHANGE PRESSURE" appears in the display. Then the potentiometer adjusts the desired pressure and by pressing the same button (B.3) saves the new selection.

During the operation of the system, the following useful messages are displayed on the LCD:
• During the process of setting the maximum actuator pressure, the current ("Current: #") and the new pressure ("New: #") are displayed on the LCD, where # numbers between 0 and 100.
• "HARD CLOSE" is displayed during the Hard Actuator closing process.
• The "HARD OPEN" message appears during the Hard Actuator opening process.
• In the closing procedure of the Soft Actuator, "SOFT CLOSE" appears on the first line of the LCD and the current Soft Actuator is pressed in the second.
• "SOFT OPEN" appears during the Soft Actuator opening process.

#Android APP

The Android application "THE BINARIES" has been developed for the needs of the system, which displays various measurements performed by the system and remote control of the opening and / or closing of the actuators.
The main features of the application are described below:

![alt text](https://github.com/robotonio/The_Binaries/blob/master/android_app_description.png)

#Microcontroller

The system controlled by the ESP32 microprocessor, witch has built in Bluetooth and WiFi capabilities, that make this electronic board not only autonomous to support automation but also compared to other platforms (such as those in the Arduino range), much more efficient and complete.

#Circuit

The following figure shows the system connection:

![alt text](https://github.com/robotonio/The_Binaries/blob/master/circuit.png)




