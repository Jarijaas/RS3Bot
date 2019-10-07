rule SetSkillXP
{
  meta:
    id = 4
    
  strings:
    $hex_string = {48 89 5C 24 08 [15-25] 48 8B 42 18 48 8B F9 4C 8B 52 10 4C 8D 48 01 4C 89 4A 18 4D 8D 41 01 }

  condition:
    $hex_string
}