# HIDAPI connectivity tests for Mac

I have written two simple test programs to test connecting to HID U2F devices on
Mac. A HID U2F device is defined as a device with usage page 0xf1d0 and usage
0x0001. The example programs are:

1. `example_hidapi.c`: Uses HIDAPI
2. `example_native.c`: Uses native APIs

To build, `cd` to `hidapi/mac` and run:

    clang -g -D_THREAD_SAFE -pthread -framework IOKit -framework CoreFoundation hid.c example_hidapi.c -I../hidapi -o example_hidapi
    clang -g -D_THREAD_SAFE -pthread -framework IOKit -framework CoreFoundation example_native.c -o example_native

A debugging printf has been added to `mac/hid.c` to log when `IOHIDDeviceOpen`
returns `kIOReturnNotPrivileged`.

## Result

In my testing, `example_hidapi` succeeds in connecting to the device
approximately half the time, and otherwise fails with `kIOReturnNotPrivileged`.

`example_native` always succeeds in connecting.

I believe that this may be an issue with HIDAPI in general, and not specific to
Yubikeys. I have also encountered the "intermittently fails to connect" behavior
when testing `example_hidapi` with a Kensington USB Keyboard (although,
interestingly, when I tried it with a Logitech USB mouse it always succeeded in
connecting). I have not yet tried `example_native` with any non-U2F devices.
