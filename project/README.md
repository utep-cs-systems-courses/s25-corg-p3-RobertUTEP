Use make load to compile and load the program.

Each of the four buttons on the expansion board controls a different
behaviour: the first button lights only the green LED, the second lights only
the red LED, the third lights both LED's and the fourth plays a buzzing sound
while it's held down.

State Machine:
Each button press changes the current mode(red, green, both, or sound)
The LEDs and buzzer react according to the active state.

The LEDs automatically dim and brighten in smooth steps over time,
