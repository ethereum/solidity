contract test { function() external { uint x = 3; int y = -4; x ** y; } }
// ----
// TypeError: (62-68): Operator ** not compatible with types uint256 and int256. Exponentiation power is not allowed to be a signed integer type.
