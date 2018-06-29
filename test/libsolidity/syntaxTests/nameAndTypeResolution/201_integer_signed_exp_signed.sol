contract test { function() external { int x = -3; int y = -4; x ** y; } }
// ----
// TypeError: (62-68): Operator ** not compatible with types int256 and int256
