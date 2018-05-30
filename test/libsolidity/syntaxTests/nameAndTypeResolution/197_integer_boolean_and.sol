contract test { function() public { uint x = 1; uint y = 2; x && y; } }
// ----
// TypeError: (60-66): Operator && not compatible with types uint256 and uint256
