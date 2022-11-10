contract test { fallback() external { uint x = 1; uint y = 2; x && y; } }
// ----
// TypeError 2271: (62-68): Built-in binary operator && cannot be applied to types uint256 and uint256.
