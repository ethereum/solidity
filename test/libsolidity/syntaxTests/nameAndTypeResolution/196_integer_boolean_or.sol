contract test { function() external { uint x = 1; uint y = 2; x || y; } }
// ----
// TypeError: (62-68): Operator || not compatible with types uint256 and uint256
