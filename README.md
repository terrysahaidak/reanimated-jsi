# Reanimated with JSI

## How it works

Currently all the JSI code is stored inside `jsi/android` folder. I'm copy-paste it during build to react-native source code, so it will be compiled together with all those cool libs/helpers RN team wrote (jsi, fbjni etc). For that reason, I'm using react-native from branch and compiling it with all the other code. Also, all the changes to Reanimated itself are done using patch-package. Check out `patches` dir. In order to update patches, run:

```bash
npx patch-package react-native-reanimated --include "ReanimatedModule.java|Callback.java|AnimatedNode.js|NodesManager.java"
```

You can add additional files with `|` delimiter.

## Development

In order to start develop, clone this repository and run `npm install`, after it run `npm run android`. It will build the app (as well as react native) from sources and install to attached device.

If you want to change something, you can make your changes in the `jsi/android` files and then you have to run `npm run clean:android` in order to clean build folder and all the cached `so` files and then just run `npm run android` to build it again.

## License

MIT
