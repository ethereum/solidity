contract test { fallback() external { uint x = 1; !x; } }
// ----
// TypeError 4907: (50-52): Built-in unary operator ! cannot be applied to type uint256.
