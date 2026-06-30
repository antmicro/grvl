# DRM connectors list sample

This native Linux/DRM sample enumerates connected DRM connectors before GRVL is
initialized. It selects the first external connector; if none are connected,
it falls back to the first built-in connector.

`eDP`, `LVDS`, and `DSI` connectors are treated as built-in displays. All other
connected connectors with a valid mode are considered external.

Build and run:

```sh
./samples/sample.sh connectors
```

Or build only:

```sh
./samples/sample.sh --build connectors
```

The process needs access to DRM and libinput devices and must run outside X11 or
Wayland, typically from a VT.
