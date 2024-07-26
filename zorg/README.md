# Instructions

## 1. Install PlatformIO

### CLI

```
$ brew install platformio
```

### VSCode

https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide

## 2. Compile/run

### CLI

```
$ cd zorg
$ pio run -e main -t upload  # uploads the main program
$ pio run -e wall -t upload  # uploads the wall program
```

If multiple boards are plugged in:

```
$ pio device list  # lists devices
$ pio run -e -main -t upload --upload-port <port from device list>
```

### VSCode

1. `Cmd+Shift+P`
2. `PlatformIO: PlatformIO Home`
3. `Open Project`
4. Navigate to `Strangling-Fruit-System/zorg/master` or
   `Strangling-Fruit-System/zorg/wall`
5. Use `Run` command in the status bar at the bottom.

See
https://docs.platformio.org/en/latest/integration/ide/vscode.html#ide-vscode.

## Fixing squiggly lines in `common` in VSCode

Use the environment switcher in the status bar to switch to either the `main` or
`wall` environment. This will update VSCode's IntelliSense based on that
environment's projects
