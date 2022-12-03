# Netee

## Setup Instructions
1. Install a modern Linux distro (tested on Ubuntu 20.0.4)  
2. Run `sh setup.sh`  

## Running
Navigate to the build directory with `cd build`.  

### Server
`./game -server -port <port>`  
The port option is not required.  

### Client
`./game -host <ip> -port <port>`  
The port option is not required.  

## Gameplay Instructions
This describes how to play the game.  

### Win Condition
A player wins if their sword is orange during an attack or lunge and collides with the opponent's body.  
A player may parry any attack or lunge to prevent getting hit if timed correctly.  

### Controls
Moving: Left and Right Arrow Keys  
Attack: Q  
Lunge: W  
Parry: E  
Feint: R  

### Description of Moves
Attack: Very quick and short forward movement of the sword  
Lunge: Very slow and long forward movement of the sword that requires a small charge up  
Parry: Can prevent hits from attacks or lunges if timed correcty  
Feint: A fake charge up, intended to bait the opponent to parry  
