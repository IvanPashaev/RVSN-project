# Audio system

Audio system using ps3soundlib, and 16 voices there is:
  0 - decoder
  1 - background music
  2..15 - sounds

## rvsn.PlayAudioFile

just playing audiofile (mp3 or ogg) on free voice slot

**Syntax**
`rvsn.PlayAudioFile("/dev_hdd0/simple_test/audio/m-e-o-w.mp3", 255)`

**Parameters**
| Param | Type | Required | Desc |
| :--- | :--- | :--- | :--- |
| `filepath` | string | Yes | audio file |
| `volume` | int | No | volume (0-255), standart is 255 |

## rvsn.SetBackgroundMusic

infinite playing audiofile on first voice slot

**Syntax**
`rvsn.SetBackgroundMusic("/dev_hdd0/simple_test/audio/Femtanyl - Weightless!.mp3", 255)`

**Parameters**
| Param | Type | Required | Desc |
| :--- | :--- | :--- | :--- |
| `filepath` | string | Yes | audio file |
| `volume` | int | No | volume (0-255), standart is 255 |
