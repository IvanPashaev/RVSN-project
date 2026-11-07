# Characters

in RVSN 16 characters slots(include 0)

## rvsn.InitCharacter

init character in characters slot

**Syntax**
`rvsn.InitCharacter(0,"SyrianClippy",255,255,255,255)`

**Parameters**
| Param | Type | Required | Desc |
| :--- | :--- | :--- | :--- |
| `id` | int | Yes | id in characters slots |
| `name` | string | Yes | name of character |
| `r`,`g`,`b` | int | Yes | color of character's name in RGBA(0-255) |
| `a` | int | Yes | transparency of character's name in RGBA(0-255) |

## rvsn.SetCharacterSprite

**Syntax**
`rvsn.SetCharacterSprite(0,"/dev_hdd0/simple_test/images/syrian_man.png", 100, "left")`

**Parameters**
| Param | Type | Required | Desc |
| :--- | :--- | :--- | :--- |
| `id` | int | Yes | id in characters slots |
| `filepath` | string | Yes | image for character |
| `scale` | int | Yes | scale in percents |
| `position` | string | Yes | position of character's sprite(left,right,center) |

## rvsn.HideCharacterSprite

**Syntax**
`rvsn.HideCharacterSprite(0)`

**Parameters**
| Param | Type | Required | Desc |
| :--- | :--- | :--- | :--- |
| `id` | int | Yes | id in characters slots |

## rvsn.CharacterSay

**Syntax**
`rvsn.CharacterSay(0,"how about babah?")`

**Parameters**
| Param | Type | Required | Desc |
| :--- | :--- | :--- | :--- |
| `id` | int | Yes | id in characters slots |
| `text` | int | Yes | text what character will says |
