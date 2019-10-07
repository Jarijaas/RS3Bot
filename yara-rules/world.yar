rule UpdateObject
{
  meta:
    id = 3
    
  strings:
    $hex_string = { 48 89 5C 24 10 48 89 74 24 20 57 48 83 EC 20 48 8B 01 }

  condition:
    $hex_string
}

rule SetGameState
{
  meta:
    id = 6
    
  strings:
    $hex_string = { 48 89 5C 24 08 [20-30] 4C 8B 42 18 48 8B 42 10 4D 8D 50 02 4C 89 52 18 45 0F B7 0C 00 }

  condition:
    $hex_string
}

rule SetGameData
{
  meta:
    id = 7
    
  strings:
    $hex_string = { 4C 89 4C 24 20 [20-30] 48 83 42 18 06 4C 8B 4A 18 48 8B 42 10 45 0F B6 44 01 FF }

  condition:
    $hex_string
}

rule UpdateNPC
{
  meta:
    id = 10
    
  strings:
    $hex_string = { 48 83 EC 28 48 8B 49 08 45 8B 00 }

  condition:
    $hex_string
}

rule LoadRegion
{
  meta:
  id = 11

  strings:
    $hex_string = { 48 89 5C 24 10 [35-45] 80 78 35 00 74 0C [10-20] 48 83 43 18 02 }

  condition:
    $hex_string
}

rule CursorWorldContextMenu 
{
  meta:
    id = 12

  strings:
    $hex_string = {
      48 8B C4 [90-110]
      44 0F 29 B8 28 FF FF FF
      41 8B F1
      45 8B E0
      8B FA
      48 8B D9
      48 8B 41 08 [7]
      48 63 81 90 00 00 00
      83 F8 FF
      75 09 [7]
      EB 08
    }

  condition:
    $hex_string
}

rule UpdatePlayers
{
  meta:
    id = 13

  strings:
    $hex_string = { 48 89 5C 24 08 57 48 83 EC 20 48 8B 41 08 48 8B FA 48 8B 98 D0 0E 00 00 48 8B CB 48 8B 83 80 80 00 00 48 89 83 88 80 00 00 }

  condition:
    $hex_string
}

rule CursorDoAction
{
  meta:
    id = 14

  strings:
    $hex_string = { 48 89 5C 24 10 48 89 6C 24 18 56 48 83 EC 20 48 8B 41 08 }

  condition:
    $hex_string
}

rule InteractNPC
{
  meta:
    id = 15

  strings:
    $hex_string = { 40 57 48 83 EC 50 48 C7 44 24 20 FE FF FF FF 48 89 5C 24 68 48 89 6C 24 70 48 89 74 24 78 49 8B E8 48 8B DA 48 8B F1 [30-40] 0F 85 ?? 01 00 00 }

  condition:
    $hex_string
}

rule DispatchNetMessage
{
  meta:
    id = 16

  strings:
    $hex_string = { 40 57 48 83 EC 50 48 C7 44 24 20 FE FF FF FF 48 89 5C 24 60 48 8B DA 48 8B F9 48 8D 54 24 38 48 8B 4B 08 E8 [4] 90 4C 8B 44 24 40 }

  condition:
    $hex_string
}

rule InteractObject
{
  meta:
    id = 17

  strings:
    $hex_string = { 40 57 48 83 EC 50 48 C7 44 24 20 FE FF FF FF 48 89 5C 24 68 48 89 6C 24 70 48 89 74 24 78 49 8B E8 48 8B DA 48 8B F1 [20-50] 4C 8B 4C 24 48 48 3B D8 48 8B 45 08 8B 50 54 0F }

  condition:
    $hex_string
}

rule ActionRegister {
  meta:
    id = 18
    multiple = true

  strings:
    $hex_string = { 48 83 EC 38 48 C7 44 24 20 FE FF FF FF [7] 48 89 44 24 40 [20] 48 8D 0D [4] 48 83 C4 38 }

  condition:
    $hex_string
}

rule MessageDefRegister {
  meta:
    id = 19
    multiple = true

  strings:
    $hex_string = { 48 8B 05 [4] 48 8B C8 48 83 C0 08 48 89 05 [4] 48 85 C9 74 0A 48 8D 05 [4] 48 89 01 C3 }

  condition:
    $hex_string
}

rule SetProperty {
  meta:
    id = 20

  strings:
    $hex_string = { 48 8B C4 44 89 40 18 57 48 83 EC 70 48 C7 40 A8 FE FF FF FF 48 89 58 08 48 89 70 10 4C 8B DA }


  condition:
    $hex_string
}

rule OpenDialog {
  meta:
    id = 21

  strings:
    $hex_string = { 4C 89 4C 24 20 44 89 44 24 18 53 57 48 83 EC 58 }


  condition:
    $hex_string
}