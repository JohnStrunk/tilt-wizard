# tilt-wizard

[![Build Status](https://dev.azure.com/johnstrunk/tilt-wizard/_apis/build/status/JohnStrunk.tilt-wizard)](https://dev.azure.com/johnstrunk/tilt-wizard/_build/latest?definitionId=1)

`tilt-wizard` is a simple Windows cli program to automatically calibrate
DirectInput-based motion sensors used for Virtual Pinball.

## Background

When building a virtual pinball cabinet, a popular option to add is a motion
sensor so that it is possible to "nudge" the table while playing and have this
real-life jostling of the table affect the movement of the virtual pinball.

When I built my table, I chose to use the Mot-Ion controller. It appears to the
computer as a gamepad with the x and y axes detecting motion of the cabinet.
Unfortunately, these units have a good bit of noise to them, are difficult to
calibrate, and their center point tends to change with temperature.

This project is my attempt to create a simple executable that automatically
calibrates the center, limits, and deadzone on-the-fly.
