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
$ cd zorg/master
$ pio run
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

Copy either `Strangling-Fruit-System/zorg/master/.vscode/c_cpp_properties.json`
or `Strangling-Fruit-System/zorg/wall/.vscode/c_cpp_properties.json` into
`Strangling-Fruit-System/zorg/common/.vscode/c_cpp_properties.json`.