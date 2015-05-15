# Dynamic trimix blender 
**This project includes both hardware description & all needed software and firmware.**

Blender is built to mix EAN and Trimix mixtures dynamically, with no more than 40% of oxygen.

Main electronic components:
* MCU: Atmega64. 16 or 32kB versions could be used with no/minimum code changes (untested).
* ADC: AD7793. Used to get accurate O2 readings.
* Any oxygen sensors with 7-25mV @ 21% O2 output could be used without any changes.

Software needed to work with schematic and board:
* [Eagle](http://www.cadsoftusa.com/download-eagle/) for schematic and board. Free version is enough.
* [FreeRouting](http://www.freerouting.net/) - was used for initial routing. Optional.
* [Gerbv](http://gerbv.sourceforge.net/) to preview gerbers. Optional - just to control them before factory processing.

I've ordered PCB's at [SeeedStudio](http://www.seeedstudio.com/service/index.php?r=pcb), but feel free to use any PCB factory with equivalent capabilities. Board size is 44x122mm.


Developed by [DiveTeam](http://diveteam.com.ua) :rowboat:
