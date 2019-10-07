rule GameMessageRecv
{
  meta:
    id = 1
    
  strings:
    $hex_string = { 48 8B C4 55 [80-90] 48 8D 4A 02 48 89 4B 18 42 0F B7 0C 0A 3D 00 01 02 03 }

  condition:
    $hex_string
}