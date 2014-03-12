TRX
===

Arduino nanos talking wirelessly using the virtualwire library and some cheap chinese TX / RX modules.

The basestation (TX) can choose which of 8 groups to activate (which will deactivate the rest)

The RX modules can choose which group to belong to, and activate acordingly.

I'm trying to use cleverness to achieve what I need using only one-way communications.

OH.. this is also my first project using the ino tool. (and plugin for sublime text 3) which is why the dependent libraries are located in the lib folders for each of the 3 ino project folders (rx, rx_sleep, and tx are 3 seperate ino projects)
