rule SetInventorySlot {
  meta:
    id = 2

  strings:
    $hex_string = { 
      4C 89 4C 24 20
      48 89 4C 24 08
      [50-100]
      44 0F B7 EA
    }

  condition:
    $hex_string
}