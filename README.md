LSDVE
=======================================

LSD Video Editor makes video editing easier :) 

##Supported Operation:
* righclick : pause/resume the video
* leftclick : on press down and press up, two points are selected
* (A)dd a new line
* (R)emove line in a area selected by the mouse
* (Q)uit & save
* (S)ave the currect frame and go to the next frame
* (U)ndo

##TODO: 
- Use polylines instead of lines for nearby lines (probably just in an area)
- Color 
- Fix problem with removing lines


##Build Sequence

install openCV as explained here: 
http://docs.opencv.org/doc/tutorials/introduction/linux_install/linux_install.html

``` 
cmake .
killall -9 lsdvideo ; make ; ./lsdvideo bird.avi  out.lsd 
```
##Output
Output is stored in a (.lsd) file with this simple format
``` 
frame:INT   <- framenumber in the original video
lines:INT   <- number of lines in this frame
line1_X1,line1_Y1,line1_X2,line1_Y2
line2_X1,line2_Y1,line2_X2,line2_Y2
.
.
.
frame:INT
lines:INT
``` 

Hamid Ebadi
