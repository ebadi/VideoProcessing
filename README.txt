// Build
// install openCV as explained here : http://docs.opencv.org/doc/tutorials/introduction/linux_install/linux_install.html
cmake .
killall -9 lsdvideo ; make ; ./lsdvideo bird.avi  out.lsd
