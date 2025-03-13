# EEC172 WQ2025 Final Project Webpage - Battleship
 Group Member: Allison Li, Jason Kao
## Description
Our prototype will be a replica of the game battleship. This game will use two OLEDs, an IR receiver to input commands as well as save the name of the player, buttons on the microcontroller to navigate the interface, and AWS to store the scores and names of players.
Each player will take turns entering coordinates using the TV remote, and they will either receive a hit or miss. Players will take turns shooting at each ship until one side has sunk all the opposing ships, resulting in a winner.
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
