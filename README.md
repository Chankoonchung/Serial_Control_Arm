# Serial_Control_Arm
Control your Arduino arm by using serial commands.<br>
## Version 2.0:<br>
Serial_Commands:<br>
`#<index>A<angle>S<speed>` --move the servo<br>
`ping`                     --get the status of servos<br>
`reset`                    --reset servos<br>
`disconnect`               --detach servos<br>
`connect`                  --attach servos<br>
`+<index>`                 --increase 10 degrees for a servo<br>
`-<index>`                 --decrease 10 degrees for a servo<br>
