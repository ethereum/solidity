contract test { function() public { uint x = 3; int y = -4; x ** y; } }
// ----
// TypeError: (60-66): Operator ** not compatible with types uint256 and int256
