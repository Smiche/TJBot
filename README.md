![tjbot spray](http://i.imgur.com/4S8BIVo.png)

A crossplateform TrickJump Bot for Wolfenstein: Enemy Territory 2.60b and ETJump

## Mod compatibility
- Linux: **all** ETJump
- Windows: ETJump 2.2.0 Alpha

## Features / Commands
- `/+jumpbot`, `/-jumpbot` enable/disable automatic mouve mouse to the perfect angle
- `/+autojump`, `/-autojump` automatic `+moveup` (jump) activation
- `/originget` diplays current origin (similar to viewpos command)
- `/originset <x> <y>` sets new desired XY origin
- `/+origin`, `/-origin` start/stop trying to move to desired XY origin
- `/angle <deg>` set new horizontal view angle to specified value
- `/startdump <filename>`, `/stopdump` to dump view angles, origin and velocity every frame into a file
- `/+viewhack`, `/-viewhack` rotates view camera by 180 degs for jumping backwards
- `/ms_print`, `/ms_reset` prints/resets horizontal maxspeed
- `/+ps_print`, `/-ps_print` enable/disable printing predicted playerstate view angles, origin and velocity every frame in the console
- `/spray` prints tjbot spray

## Usage
- Enable `/+sprint` to make sure to always sprint, the tjbot does not work while not sprinting
- Bind a key to enable jumpbot: `/bind shift +jumpbot`, or if you want a toggle key
  ```
  set jmp1 "set jmp vstr jmp2; +jumpbot; echo ^ej^7umpbot ^eon"
  set jmp2 "set jmp vstr jmp1; -jumpbot; echo ^ej^7mpbot ^eoff"
  set jmp "vstr jmp1"
  bind MOUSE2 "vstr jmp"
  ```
- Set your FPS to 125 and enable fixed player move
  ```
  com_maxfps 125
  set pmove_fixed 1
  ```
- TrickJump!
  

(You can examine example script `jump.cfg` to see how to script)

### Ideas list
- Move scripting from engine to hack because when the engine is evaluting a
script it can not be stopped etc
- Add `relangle` (relative angle) command (current `angle` command can set only absolute view
angle)

### Bugs
- There are probably some minor bugs because I did not have time to do complete
testing so far
- Ice calculations are wrong, jumpbot is always one frame late

### Linux Dependencies
#### 64 bits only
```sh
sudo apt install gcc-multilib
```

```sh
sudo apt install gcc
```

### Build
#### Windows
Open `tjbot.sln` with Visual Studio

#### Linux
```sh
cmake .
make
```

### Run
#### Windows
Inject `tjbot.dll` to `et.exe` with the DLL injector of your choice  
You can also use `tjbot.exe` in the Release section, you just need to run it before running ET

#### Linux
```sh
./run.sh
```

### Licence
GPLv2

### Credit
setup