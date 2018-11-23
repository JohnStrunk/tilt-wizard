# General notes

## DirectInput

- Most aspects are fairly documented starting here: [DirectInput](https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee416842%28v%3dvs.85%29)
- Typical flow is to create a `IDirectInput` object and use that to create the
  `IDirectInputDevice` itself.
- On the device, `GetProperty` and `SetProperty` can be used to manipulate the
  calibration via `DIPROP_CALIBRATION` and `DIPROP_DEADZONE`.
- For `DIPROP_CALIBRATION`, the docs don't describe the structure that gets
  passed. However, it can be found in `dinput.h`:

  ```c
  typedef struct DIPROPCAL {
    DIPROPHEADER diph;
    LONG         lMin;
    LONG         lCenter;
    LONG         lMax;
  } DIPROPCAL, *LPDIPROPCAL;
  typedef const DIPROPCAL *LPCDIPROPCAL;
  ```
