# mp3NoiseMachine
Arduino noise machine and night light using DFRobot mini mp3 player and fastLED

Mp3 files are loaded onto a microSD card and read by the mp3 player module. A potentiometer is used to control the volume and a button is used to change which noises
are being played. 

Volume max is 30, but the mapped range is limited to 25 to prevent any accidental damage to speakers from turning the potentiometer too far.

LED pulses red as a night light and after 4 hours the Arduino goes to sleep to conserve power.

In the future a button (or maybe a switch) will be added to power on/off without having to remove power input 

printDetail void function was taken directly from DFRobot example code 
