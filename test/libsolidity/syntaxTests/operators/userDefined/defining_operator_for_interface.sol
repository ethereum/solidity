using {f as +} for I;

function f(I, I) pure returns (I) {}

interface I {}
// ----
// TypeError 5332: (7-8): Operators can only be implemented for user-defined value types.
