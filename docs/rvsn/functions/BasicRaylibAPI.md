# Basic raylib api implemented in RVSN

## rvsn.InitWindow

Initialize window context, like InitWindow in raylib or tiny3d_Init in Tiny3d

**Syntax**
`rvsn.InitWindow("my visual novel idk")`

**Parameters**
| Param | Type | Required | Desc |
| :--- | :--- | :--- | :--- |
| `title` | string | Yes | title for window(idk why its have this, because it dont shown on real console and emulator)|

## rvsn.DrawText

Drawing text with default raylib font

**Syntax**
`rvsn.DrawText("hello to acid ps3 server!", 100, 20, 32, 255, 255, 255, 255)`

**Parameters**
| Param | Type | Required | Desc |
| :--- | :--- | :--- | :--- |
| `text` | string | Yes | just text without formatting!|
| `x` | int | Yes | x coordinates|
| `y` | int | Yes | y coordinates|
| `font_size` | int | Yes | font size in px|
| `r`,`g`,`b` | int | Yes | color in RGBA(0-255)|
| `a` | int | Yes | transparency in RGBA(0-255)|
