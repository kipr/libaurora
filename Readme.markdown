libaurora
=========

libaurora, named after [Aurora](https://en.wikipedia.org/wiki/Aurora), is a graphics library with a Daylite interface.
This allows other Daylite nodes to forward key and mouse input to and receive frames from programs using libaurora.

Requirements
============

* CMake 2.8.0 or higher
* libpng
* zlib

Building (OS X and Linux)
=========================

    cd libaurora
    mkdir build
    cd build
    cmake ..
    make
    make install

Published Daylite Topics
========================
`/aurora/frame`:
```
"msg":
{
  "format": <utf8 string; format of the frame (e.g. PNG) >,
  "width": <int32; width of the frame in pixel>,
  "height": <int32; height of the frame in pixel>,
  "data": <binary; raw image data>
}
```

Subscribed Daylite Topics
=========================
`/aurora/key`:
```
"msg":
 {
   "key_pressed": [ <key_code; key codes of all currently pressed keys>, ... ]
 }
```

*Note:* A key is considered to be pressed until a `/aurora/key message` without this key is received.

`/aurora/mouse`:
```
"msg":
{
  "pos": {
    "x": <int32; x position of the mouse pointer in pixel>,
    "y": <int32; y position of the mouse pointer in pixel>
  },
  "button_down": {
    "left": <bool; true if the left mouse button is down>,
    "middle": <bool; true if the middle mouse button is down>,
    "right": <bool; true if the right mouse button is down>
  }
}
```

*Note:* It is not required that all of the fields are set. E.g. `"msg": {"button_down": {"left": true}}` just sets the left mouse
button and keeps all the other values untouched.

Authors
=======

* Stefan Zeltner

License
=======

libaurora is released under the terms of the GPLv3 license. For more information, see the LICENSE file in the root of this project.
