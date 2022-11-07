contract test { fallback() external { uint x = 3; int y = -4; x ** y; } }
// ----
// TypeError 2271: (62-68): Built-in binary operator ** cannot be applied to types uint256 and int256. Exponentiation power is not allowed to be a signed integer type.
