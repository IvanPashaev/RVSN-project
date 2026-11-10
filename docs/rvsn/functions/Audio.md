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

## rvsn.StopSound

stop playing audio in slot

**Syntax**
`rvsn.StopSound(2)`

**Parameters**
| Param | Type | Required | Desc |
| :--- | :--- | :--- | :--- |
| `voice` | int | Yes | voice id |

## rvsn.SetBackgroundMusic

infinite playing audiofile on first voice slot

**Syntax**
`rvsn.SetBackgroundMusic("/dev_hdd0/simple_test/audio/Femtanyl - Weightless!.mp3", 255)`

**Parameters**
| Param | Type | Required | Desc |
| :--- | :--- | :--- | :--- |
| `filepath` | string | Yes | audio file |
| `volume` | int | No | volume (0-255), standart is 255 |

## rvsn.StopBackgroundMusic

stop playing background music(slot 1)

**Syntax**
`rvsn.StopBackgroundMusic()`

## rvsn.PauseBackgroundMusic

pause background music(slot 1)

**Syntax**
`rvsn.PauseBackgroundMusic()`

## rvsn.UnpauseBackgroundMusic

unpause background music(slot 1)

**Syntax**
`rvsn.UnpauseBackgroundMusic()`
