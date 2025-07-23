
# SimpleUI

This is a versatile library for creating GUIs, made to be user-friendly, have simple syntax and be memory efficient. With SimpleUI, you can easily animate objects, images, icons, and thanks to the advanced focusing system, you only have to bind navigation inputs and the library will handle the focusing automatically, supporting every bizzare layout.



## Documentation

Not ready yet.


## Demo
![A screenshot of a homepage](https://github.com/alex-makes-things/esp32-ui/blob/main/src/images/screenshot.png)


## Optimizations
The library works in a non-destructive way, meaning you declare every Ui element yourself.
The inner workings make sure to be memory-friendly by using mostly pointers to avoid the duplication of large arrays or objects, and memory leaks are avoided by making sure that the memory allocated by the library is always freed.
SimpleUI makes extensive use of the C++ standard library, leveraging unordered maps, vectors, mathematical functions, initializer lists and more.


## Authors

- alex-makes-things


## Features

- Animations
- Reliable focusing system
- Wide gamma of ui elements
- Blazingly fast, most scenes's framebuffers can be calculated in under 1ms (Tested with a resolution of 128x64).
- Timers



