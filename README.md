# Serial_Control_Arm
Control your Arduino arm by using serial commands.<br>
## Version 1.0:<br>
* Excemple command:<br> 
`#1P2000T2000!#2P750T2000!#3P600T2000!#5P600T2000!`<br>
## Version 2.0:<br>
* Changelog:<br>
Add ["VarSpeedServo.h"](https://github.com/netlabtoolkit/VarSpeedServo) to control the servos.<br>
* Serial_Commands:<br>
`#<index>A<angle>S<speed>` --move the servo<br>
`ping`                     --get the status of servos<br>
`reset`                    --reset servos<br>
`disconnect`               --detach servos<br>
`connect`                  --attach servos<br>
`+<index>`                 --increase 10 degrees for a servo<br>
`-<index>`                 --decrease 10 degrees for a servo<br>
