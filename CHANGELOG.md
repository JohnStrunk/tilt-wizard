# Changelog

## Master

- Added
  - Axis range can be customized via `-r range`.
  - Moving average momentum can be customized via `-m value`.
  - `direader.exe` to help diagnose add debug DirectInput settings
- Changed
  - Command line has changed to use switches. (`-l` to list devices and `-d
    guid` to tune a particular device).
  - Output now shows recommended deadzone for each axis instead of the standard
    deviation of incoming data.
- Limitations
  - Deadzone settings are not visible to other applications and must be set
    manually.

## Version v0.1

- Added
  - Initial release.
  - Ability to list available devices.
  - Ability to automatically tune center point of X & Y axes.
