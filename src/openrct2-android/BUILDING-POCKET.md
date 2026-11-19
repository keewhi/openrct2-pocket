# Building OpenRCT2 Pocket for Android

The Android app is upstream OpenRCT2's Android project, set up to run Pocket: the handheld UI is
always on, and the app (`io.openrct2.pocket`) installs beside upstream OpenRCT2. Pocket is laid out
for a 640×480 screen (the R36S) and scales up on bigger panels.

## The easy way

Use OpenRCT2's Android build image, which has the SDK, NDK, CMake and nlohmann-json versions
`app/build.gradle` expects. From the repository root:

```sh
docker run --rm -v "$PWD":/src -w /src ghcr.io/openrct2/openrct2-build:26-android \
  bash src/openrct2-android/ci-build.sh OpenRCT2-Pocket.apk
```

This is what the testing and release workflows run (`.github/workflows/android-apk.yml`).

## Without the image

You need JDK 17 or newer, the Android SDK with the NDK and CMake versions named in
`app/build.gradle` (`sdkmanager "ndk;27.3.13750724" "cmake;4.0.3"`), `jq`, and `gxc` from
[libsawyer](https://github.com/OpenRCT2/libsawyer/releases) on `PATH` (`ci-build.sh` fetches it if
it is missing). nlohmann-json is fetched by CMake when `/opt/openrct2/include` does not have it.

```sh
cd src/openrct2-android
echo "sdk.dir=$ANDROID_HOME" > local.properties
./gradlew assembleRelease            # app/build/outputs/apk/release/app-release.apk
./gradlew assembleRelease -PabiFilter=arm64-v8a   # quicker: the R36S family is arm64
```

Useful properties: `-PpocketVersion=` (version name), `-PpocketVersionCode=` (must grow for an
update to install), `-PabiFilter=` (comma-separated ABIs; the default is `armeabi-v7a,arm64-v8a`).

Without `OPENRCT2_KEYSTORE_FILE` and `OPENRCT2_KEYSTORE_PASSWORD`, the APK is signed with the
debug key in `app/external/`, so test builds install over each other. That key is public: sign
published releases with your own.

## Minimum Android version

Android 7.0 (API 24). OpenRCT2's and Pocket's own code builds for API 21, but the prebuilt
dependencies (openrct2-dependencies-android v13) use libc functions that only exist from API 24.
Going lower means rebuilding those dependencies for API 21.

## On the device

- The user folder is `openrct2-pocket/` on the internal storage (`--user-data-path`). On first
  launch it gets `config.ini` from `app/src/main/assets/pocket/config.ini`; an existing one is
  never replaced.
- RCT2 files are looked for, in order, in `openrct2-pocket/RCT2`, `rct2` and `RCT2` on the
  internal storage and then on each SD card. The first folder with `Data/g1.dat` is passed as
  `--rct2-data-path`. When none has it, the app explains where to copy the files instead of
  starting the game.
- `GameActivity` sets `OPENRCT2_HANDHELD=1`, `SDL_ANDROID_TRAP_BACK_BUTTON=1` (the system Back
  key reaches the game, where Pocket treats it as B) and `SDL_RENDER_SCALE_QUALITY=0`.
- The park is saved when the app goes to the background, as Android may end a backgrounded app
  without warning, and when the system closes it. Leaving the game ends the process, so the next
  launch starts clean.
