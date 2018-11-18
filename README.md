# tilt-wizard

[![Build Status](https://dev.azure.com/johnstrunk/tilt-wizard/_apis/build/status/JohnStrunk.tilt-wizard)](https://dev.azure.com/johnstrunk/tilt-wizard/_build/latest?definitionId=1)

`tilt-wizard` is a simple Windows cli program to automatically calibrate
DirectInput-based motion sensors used for Virtual Pinball.

## Background

When building a virtual pinball cabinet, a popular option to add is a motion
sensor so that it is possible to "nudge" the table while playing and have this
real-life jostling of the table affect the movement of the virtual pinball.

When I built my table, I chose to use the [Mot-Ion
controller](http://www.nanotechgaming.com/mot-ionkit.php). It appears to the
computer as a gamepad with the x and y axes detecting motion of the cabinet.
Unfortunately, these units have a good bit of noise to them, are difficult to
calibrate, and their center point tends to change with temperature.

This project is my attempt to create a simple executable that automatically
calibrates the center, limits, and deadzone on-the-fly.

## Using tilt-wizard

*Well, you can't, yet... It's not done.*

What's the status?

- [x] Enumerate the DirectInput devices attached to your computer by running the
  executable w/ no options.
- [x] Open and read the x & y axes of a device specified via its UUID.
- [ ] Automatically adjust the center point via an EMA of measured values and
  set the axis limits a fixed value on each side of the calculated center point.

## Building from source

You will need [MinGW-w64](http://www.mingw-w64.org/) to build the source. While
you can download and install it directly, I recommend using
[Chocolatey](https://chocolatey.org/). Chocolatey is [easy to
install](https://chocolatey.org/install#install-with-cmdexe) and once you get it
running, you can install MinGW-w64 via:

```
C:\>choco install mingw
...
```

Once you have the prerequisites installed, make sure you update your PATH. In my case, that was:

```
set PATH=%PATH%;C:\ProgramData\chocolatey\lib\mingw\tools\install\mingw64\bin
```

You should now be able to build via:

```
c:\tilt-wizard> mingw32-make
... build output ...
```

Then run it via:

```
c:\tilt-wizard> tilt-wizard.exe
```
