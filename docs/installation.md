# Installation

This document provides installation guidance for integrating tilt-wizard with
your pinball setup.

## PinballX

Integrating with [PinballX](https://www.pinballx.com/) is fairly
straightforward. Make a note of:

- the directory where you have placed the `tilt-wizard.exe` Program
- the GUID of the device you want to automatically calibrate

Launch the PinballX configuration program and go the "Startup settings" menu.
Configure tilt-wizard to run on startup as shown below. Substitute the directory
of `tilt-wizard.exe` for the "Working path" and put your device GUID in the
"Parameters" field.

![PinballX startup settings](images/pinballx_config.png)
