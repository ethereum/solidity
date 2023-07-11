using {f as +} for I global;

function f(I, I) pure returns (I) {}

interface I {}
// ----
// TypeError 8841: (0-28): Can only use "global" with user-defined types.
// TypeError 5332: (7-8): Operators can only be implemented for user-defined value types.
