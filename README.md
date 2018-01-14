# Zumo Search and Rescue Assignment
## https://github.com/jakesgit/progthings

### Tasks Achieved
Out of the tasks that were outlined in the brief, my robot successfully completed the following:
- Task 1
- Task 2
- Task 3
- Task 4A, 4B and 4C

It does not complete task 5, although it does store relevant room/corridor data that could be used to achieve this task with. 

[**Click here to view demonstration video**](https://www.youtube.com/watch?v=NTTpULcHOJQ)

### Acknowledgement and Sources

My robot design uses libraries from provided by Pololu, the creators of the Zumo. 

These included:

- ZumoBuzzer - for audio feedback	
-	ZumoMotors - for controlling the motors of the Zumo
- Pushbutton - for operating a button on the back of the Zumo
- ZumoReflectanceSensorArray - for reading values of the detector on the underside of the Zumo	

I also used a library called NewPing, created by Tim Eckel. It adds better performance and more features to the ultrasonic sensors used for detecting objects by the Zumo.

My project was loosely based on the BorderDetect example provided by Pololu for demonstrating how the ZumoRefelectanceSensorArray works.

### Development

##### ZumoReflectanceSensorArray
One of the biggest issues I faced during this task was simply deciding the best way to achieve what I wanted to. The brief pointed out that we should check the BorderDetect and LineFollower examples provided by Pololu to understand the reflectance sensor, which I did. At first I didn't understand the differences between ZumoReflectanceSensorArray::read() method (used in BorderDetect) and ZumoReflectanceSensorArray::readLine() method (used in LineFollower). They both took an array of integers as the parameter, but one required callibration, one did not, and both behaved completely differently. After a lot of digging (SensorCallibration example) I learnt that readLine method was simply returning an int value of 0-3500 to indicate how close to the line it was (that it knows from calibration), whereas read() returns the QTR threshold value for the specific sensor. 
I chose to base my own work on the BorderDetect example and less so on the LineFollower, as I felt it suited my needs better and I liked the fact that callibration was not necessary (I found callibration to be very hit and miss).

##### Stopping at a "dead-end"
Using the method outlined above to read the raw sensor values, whilst at the time I thought would have been the most efficient choice, later came back to bite me whilst trying to complete task 3, which outlines that the Zumo should stop when it reaches a corner/dead-end. I thought this would be simple, read the leftmost & rightmost values, and if they both return a value above our threshold, stop the Zumo. This proved to be near on impossible as unless the Zumo was perfectly straight, both sensor reading would never return as above the treshold at exactly the same time, and thus the stop condition would be skipped but the turn condition (if leftmost or rightmost sensor detects, turn the other way) would be true, which means it would turn away from the line and continue, never successfully stopping. I managed to overcome this by changing the logic so that if the leftmost sensor detected a line and the rightmost does not, move slightly to the left and then re-read the values before deciding what to do (and vice versa for the other side).

##### Converting code to “typical” OO
At first I did not plan to use classes for my code, but as the amount of flags I was using grew and grew, I started to regret my decision. I then re-read the brief and discovered that I had to store room/corridor data in order to achieve the goals outlined in task 5, so I decided to essentially rewrite my program, this time using Corridor and Room classes to hold useful data. 

##### Turning
Initially I decided that I wanted to set the Zumo up to be fully automatic, using its surroundings to detect which way it is able to turn. The thing is, to do this, the Zumo would need to be able to perform perfect 90 degree turns. I initially tried to do this by saying "rotate left at x speed for y milliseconds", but this wasn't plausible due to the inconsistencies in motor speed of the Zumo. I can only assume this was caused by the battery life of the robot, as when putting in new batteries the motors are significantly more powerful for a short period.
 I then decided to spend a few hours trying to set up the Zumo's gyro using a library provided by Pololu called L3G. After tinkering with some math and making little progress, and re-reading the brief, I decided that it probably wasn't worth the effort as little or no marks would be allocated for doing such a thing, so I moved on.
The Zumo now disconnects from the border detection logic when hitting a wall and allows the user to turn it manually using the GUI, as requested in the spec.

##### Ultrasonic Sensor
I lost more time than I care to admit trying to overcome a silly issue with the US sensor. As previously mentioned I used the NewPing library to handle the sensor, and to do so successfully requires a NewPing constructor to be called with 3 ints as parameters, the trigger pin, the echo pin and the desirable max scanning distance of the sensor. I set everything up, and discovered that the sensors would only return a reading of constant 0s (ie, nothing detected). I naively assumed that because I was getting readings, even if it was only 0s, things were set up ok and that it must have been a faulty sensor. In actual fact, I had put the trigger and sensor pin values the wrong way round in the NewPing constructor. 
