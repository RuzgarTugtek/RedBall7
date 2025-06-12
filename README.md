## How to Run This Game

This game was developed using the [raylib](https://www.raylib.com/) library. Please make sure you have raylib installed correctly before compiling.

### Requirements
- C compiler (like GCC or Clang)
- raylib library (version 4.0 or above is recommended)
- Make sure your system supports OpenGL

### Compilation Instructions
If you're using GCC, compile the game with:

```bash
gcc -o MyGame main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
Note: You might need to adjust the libraries depending on your OS. On Windows, link against raylib.lib and use the appropriate setup.
Make sure the raylib include and library paths are correctly set in your IDE or compiler configuration.

If you're using Visual Studio, Code::Blocks, or another IDE, refer to the raylib wiki for platform-specific setup guides.

Enjoy the game!
