# Animated GIF sample

This sample demonstrates:

- an animated GIF loaded as `ImageContent`,
- enabling and disabling playback,
- enabling and disabling looping,
- restarting playback from JavaScript.

JS API used:

```js
image.animationEnabled;
image.animationLoop;
image.activeFrame;
image.RestartAnimation();
```

When looping is disabled, pressing **Restart animation** plays the GIF once and
leaves it on its final frame.

The two `SwitchButton` controls use `onSwitchON` and `onSwitchOFF`, while the
restart control is a regular `Button`.
