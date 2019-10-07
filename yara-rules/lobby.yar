rule SetLobbyNews
{
  meta:
    id = 8
    
  strings:
    $hex_string = { 48 8B C4 [35-45] 48 89 58 08 48 8B FA 4C 8B F1 0F 57 C0 F3 0F 7F 44 24 20 }

  condition:
    $hex_string
}

rule SetWorlds
{
  meta:
    id = 9
    
  strings:
    $hex_string = { 48 83 EC 28 41 8B 00 48 8B 49 08 FF C8 4C 63 C0 }

  condition:
    $hex_string
}