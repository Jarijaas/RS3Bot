rule SetAccountSecuritySettings
{
  meta:
    id = 5
    
  strings:
    $hex_string = {48 8B C4 [15-25] 48 89 58 08 48 89 70 10 4C 8B D2 48 8B F1 48 83 42 18 02 }

  condition:
    $hex_string
}