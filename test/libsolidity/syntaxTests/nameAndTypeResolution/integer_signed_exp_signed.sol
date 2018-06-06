contract test { function() public { int x = -3; int y = -4; x ** y; } }
// ----
// TypeError: (60-66): Operator ** not compatible with types int256 and int256
