contract test { function() public { uint x = 3; int y = -4; y ** x; } }
// ----
// TypeError: (60-66): Operator ** not compatible with types int256 and uint256
