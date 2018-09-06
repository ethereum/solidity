contract test { function() external { uint x = 3; int y = -4; y ** x; } }
// ----
// TypeError: (62-68): Operator ** not compatible with types int256 and uint256
