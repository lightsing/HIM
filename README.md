# Maze Runner [A Simple CG Project]

A simple level-up maze game built only upon the GLFW library (no game engine).

![demo_level](docpic/demo_level.png)

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
-   `FreeType`

Project developed and tested under CLion. Models are generated from Blender and loaded to OpenGL environment through `Assimp`. Texture images partially retrieved from Minecraft packages and are loaded through `stb_image`. Fonts are loaded using `FreeType`.

<br>

## Getting Started

### CLion

With CLion, open this project and set the `cmake` toolchain as VS.

Then build and run at ease.

---

### CMake

To use `Cmake`, make sure to `cmake` with VS configurations.

Then build the solution to generate an executable in the `bin` folder.

<br />

## Basic Game Logic

#### Simple Design

Two controllable **characters**:

-   Adventurer: moving in the maze to find a way out
-   UAV: An extra view deployed by the adventurer, which can fly in the sky and phase through walls to help the adventurer find the way out

The player can switch between two characters using keyboard inputs <kbd>1</kbd> (Adventurer) and <kbd>2</kbd> (UAV). The characters can restart at the start of the maze using keyboard input <kbd>R</kbd>.

<table>
    <tr>
    	<td><img src="docpic/demo_adventurer.png" /></td>
    	<td><img src="docpic/demo_uav.png" /></td>
    </tr>
</table>

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

---

#### More Details

In each game level, the adventurer is placed right outside the entrance of the maze. The adventurer is allowed to move freely outside the maze but once he (it?) enters the maze, the clock starts ticking.

<table>
    <tr>
    	<td><img src="docpic/demo_level.png" /></td>
    	<td><img src="docpic/demo_start.png" /></td>
    </tr>
</table>

The adventurer will be running in the maze then, trying to find the exit. Once the adventurer passes the exit and reaches outside, the corresponding game level is finished. The adventurer can enter <kbd>r</kbd> to level up to the next round (if the adventurer has not finish the level, he can also use <kbd>r</kbd> to restart the current level).

![demo_win](docpic/demo_win.png)

<br>

## Advanced Gameplay

To win the game with as little time as possible, the player can do the following things which will make getting lost in the maze more fun:

-   Asking for the UAV’s assistance

The UAV in the game is actually a “powerful” UFO above the adventurer, that can fly freely in the sky and even phase through walls :) . The player can switch to the UFO’s view by entering keypad <kbd>2</kbd> and use <kbd>1</kbd> to switch back to the adventurer’s. The UAV’s view will aid the player with finding a right way out on the full scale.

However, it is kindly reminded that there will be time penalty if the UAV is used too often.

![demo_uav_assistance](docpic/demo_uav_assistance.png)

-   Mark a wall block

You might want to mark down the path you have taken. The UAV can also mark the wall to point a direction for the adventurer. To mark a wall block, simple aim at it and enter <kbd>e</kbd>. If the current wall is the marked one, it will be removed as a mark instead.

<table>
    <tr>
    	<td><img src="docpic/demo_wall_marked.png" /></td>
    	<td><img src="docpic/demo_wall_mark_removed.png" /></td>
    </tr>
</table>

-   Collect tiny boxes to reduce time consumption

There are three boxes randomly distributed in the maze. Picking any of them will give you a reduce of time (a “bonus”). It will be comforting if you’ve reached a dead end but then managed to find a box with a bonus of 50 seconds (not entirely a waste of time, isn’t it?)

<table>
    <tr>
    	<td><img src="docpic/demo_collection.png" /></td>
    	<td><img src="docpic/demo_collection_collected.png" /></td>
    </tr>
</table>

<br>

## Technical Support

We do not use a game engine, thus some features are hard to implement, such as collision detection and shadowing. After tons of debugging, we reached the effect as follow:

-   Collision Detection

![demo_collision](docpic/demo_collision.png)

The adventurer’s model is basically a ball. It is definitely designed to be blocked by walls. However, we currently use point to plane detection method. Therefore the ball might appear partially in the wall. With certain floating-point value misses on the edges of the wall blocks, the ball might phase through walls (very rarely, but it does happen sometimes). We are still trying to find out why.

-   Shadow

We use depth test and shadow texture mapping to implement shadowing from a point light source. This takes us a lot of time and it is difficult to debug a GLFW application… But anyway we made it here. The result is roughly as follow:

![demo_0](docpic/demo_0.png)

![demo_shadow](docpic/demo_shadow.png)

<br>

## Debugging Option

-   <kbd>B</kbd>: bind / unbind the UAV with the adventurer
-   …