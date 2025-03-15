# EEC172 WQ2025 Final Project Webpage - Battleship
 Group Member: Allison Li, Jason Kao
## Description
Our prototype will be a replica of the game battleship. This game will use two OLEDs, an IR receiver to input commands as well as save the name of the player, buttons on the microcontroller to navigate the interface, and AWS to store the scores and names of players.
Each player will take turns entering coordinates using the TV remote, and they will either receive a hit or miss. Players will take turns shooting at each ship until one side has sunk all the opposing ships, resulting in a winner.
## Implementation
![Image](https://github.com/user-attachments/assets/a16c0152-69e7-44d2-9a4c-8898be21b665) <br />
To start off, we needed to program the remote so that it is possible to decode the signal from the remote and obtain what letter was inputted. The circuitry we used is shown above for the IR sensor. The way we were able to get the signal was by using an interrupt handler that would check how long the time was between each falling edge. This would allow us to interpret the entire signal the TV remote sends to the input pin. Then, using that signal we would be able to decode which letter was pressed on the remote. To cycle through the letters, we implemented a way for the letter to be changed when the “1” button is clicked on the remote.  

The next part is implementing the switches. This part is relatively simple, we just had a couple lines of codes that would poll for when the switch is pressed. If it was pressed, then the game would start. Another functionality we coded in was for the scores to be sent to AWS using the switch.

The OLED was connected to the CC3200 using SPI and the wiring was done according to the pins we sent in the pin_mux_config.c file. To start up the OLED, we used the normal initialization commands along with the other files that we created in lab 2. 

The next part is making sure the AWS is able to send emails correctly. To do this, we took code from lab 4 and used it in order to format the information that we wanted to send. Along with this, we programmed a button that we would use to send the information across UART. This was done because the games ended at different times on each screen. So, the buttons ensured that the sending and receiving over UART was done at the same time.

The last part of the implementation was drawing the game on the OLED and figuring out all of the game logic. I first started out by drawing two 7x7 boards, one on the top left and one on the bottom right. I wrote in prompts for the user in the bottom left corner. Then according to the prompts the users would type in the necessary information to input their username and ship location. Furthermore, if the user were to enter something wrong, there is error checking that would tell them the error and ask them to try again. Next, inside of the enter key, I would draw the ship into the top left board. This board will be where the user’s ships reside. I created 3 ships, using 3 different while loops. All the ships had different lengths, one of 2, 3 and 4 length.

The next part was creating another while loop where I could poll the signal from the IR remote. This is for the attacking portion. Here, I would also poll for specific UART inputs from the other device like HIT, MISS, or a coordinate to attack. The way the code works is that the first coordinate entered will be sent over UART to the other device. The device will decide whether it was a HIT or a MISS, and then send HIT or MISS back through UART. Then, the original device will note that it was a HIT or a MISS and mark the original square as red or green. This was a continuous poll, and I kept track of the ships using an array. When ships were hit, I would delete them off the array, and when the array was empty, the game would end.

## State Diagram
![Image](https://github.com/user-attachments/assets/622042eb-228b-4c15-b635-95226725edf4)
**S1:** Idling, initialize the peripherals. Set up OLED and CC3200. Start page of the game, press *SW3* to start the game. <br />
**S2:** Game startas, asks user to input ship locations. There are 3 different length of ships in total. <br />
**S3:** Place ships at specified locations, use *Mute* to finish placing ships and move to next state.<br />
**S4:** Actual game starts, ask player 1 where they want to fire.<br />
**S5:** Place red marker where player 1 fired, and show on player 2's screen that his/her ship has been hit.<br />
**S6:** Place green marker where player 1 fired, and show on player 2's screen that his/her ship has been missed, and where it missed.<br />
**S7:** Ask player 2 where they want to fire.<br />
**S8:** Place red marker where player 2 fired, and show on player 1's screen that his/her ship has been hit.<br />
**S9:** Place green marker where player 2 fired, and show on player 1's screen that his ship has been missed, and where it missed.<br />
**S10:** Game finished, send result and display win or lose on player's screen, press *Reset* to play again. <br />
## System Architecture Diagram:
![Image](https://github.com/user-attachments/assets/e2757739-548c-4865-aa1c-c3d6e6e25c5b) <br />
The IR receiver will capture the signal from the remote control and send it to the microcontroller; program the microcontroller so that it can decode the signal and get the correct username and input coordinates from the player. The *SW3* button will behave as a start button and send button; when players press *SW3*, the game will start; at the end, when players press *SW3*, they will send the scores and username to the other player. Program the microcontroller so that it can read the data correctly and implement the logic of the battleship game. The OLED will display the game; program the microcontroller so that the game will continuously update on the OLED; two microcontrollers will communicate with each other via UART. Lastly, press *1* on AT&T TV remote control to send the score and username to the AWS, and get game result via email.
## Design specifications
**Core Component:** CC3200 microcontroller <br />
**Hardware Communication Protocols:** SPI for communication with OLED, UART for communication between two CC3200 microcontrollers <br />
**Web Service:** AWS IoT for sending email <br />
**Sensing Devices:** IR receiver for input username and coordinates, SW3 button for start the game and send result. <br />
## Project Video Demo
Here is the link for the demo video <br />
https://drive.google.com/file/d/1PDvoV8rTZNFeHTsMCljDYeIdwGIUFCpB/view?usp=sharing

