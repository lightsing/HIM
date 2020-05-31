# CG Project [Maze Runner]

A simple maze game built only upon the GLFW library (no game engine).

<table>
    <tr>
    	<td><img src="docpic/demo_0.png" /></td>
    	<td><img src="docpic/demo_1.png" /></td>
    </tr>
</table>

<br>

## Project Settings

-   OpenGL with version > 3
-   GLFW Library
-   GLM Library
-   `stb_image` Library
-   `Assimp`
-   Blender

Project developed and tested under CLion. Models are generated from Blender and loaded to OpenGL environment through `Assimp`. Texture images partially retrieved from Minecraft packages and are loaded through `stb_image`.

<br>

## Getting Started

### CLion

With CLion, open this project and set the cmake toolchain as VS.

Then build and run at ease.

---

### CMake

To use Cmake, make sure to cmake with VS configurations.

Then build the solution to generate an executable in the `bin` folder.

<br />

## Basic Game Logic

Two controllable **characters**:

-   Adventurer: moving in the maze to find a way out
-   UAV: An extra view deployed by the adventurer, which can fly in the sky and phase through walls to help the adventurer find the way out

The player can switch between two characters using keyboard inputs <kbd>1</kbd> (Adventurer) and <kbd>2</kbd> (UAV). The characters can restart at the start of the maze using keyboard input <kbd>R</kbd>.

**Movement**:

-   <kbd>W</kbd>: Forward
-   <kbd>S</kbd>: Backward
-   <kbd>A</kbd>: Left
-   <kbd>D</kbd>: Right
-   <kbd>Space</kbd>: Fly up (only UAV)
-   Left <kbd>Ctrl</kbd>: Fly down (only UAV)

With the left <kbd>Shift</kbd> held, the characters can move in a faster speed.

**View**:

-   By rotating the mouse or the mouse wheel, the view can be adjusted

**Winning State**

The player wins the game by reaching the exit of the maze.