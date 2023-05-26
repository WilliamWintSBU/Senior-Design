# Senior-Design
Senior design project for the Solar Racing Team at Stony Brook University

GUI has python code for reciving data on shore

STM32F072b-DISO code has code that runs on microcontroller. It is in a CUBE IDE project file. CUBE IDE is provided by the chip manufauter (STM) and should be used for changing this code. It also austogenerates certain files. This folder also has doxygen documentaion. Click on index.html in the folder html to see a local html page with doccumentation.

We also used another STM32 discovery board for prototyping. It is called an STM32F0-Discovery board, and has an stm32-F0 family microcontroller that is similar to the one used in the final design, but that is differtent in inportant ways. It has a seperate codebase with an earlier version of the code. This code will not work with the STM32-F072 Discovery board.

# Arduino
It may also be possible to program the microcontroller board using the Arduino IDE. See this link: https://github.com/stm32duino/Arduino_Core_STM32. We did not do this because we did not know it was possible, and because we thought that using an Arduino would not be apropriate in a senior design project. This may or may not have been hubris on our part. It is worth looking into if you need re write the code and can't get the CUBE IDE to work, but it will mean starting from scratch. Consider this before you overwrite the firmware on the board. Or you could switch the microcontroller to one you feel more comfortable with.
