contract test { fallback() external { uint x = 1; !x; } }
// ----
// TypeError: (50-52): Unary operator ! cannot be applied to type uint256
