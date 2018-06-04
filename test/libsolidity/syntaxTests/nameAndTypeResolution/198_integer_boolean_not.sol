contract test { function() public { uint x = 1; !x; } }
// ----
// TypeError: (48-50): Unary operator ! cannot be applied to type uint256
