type M is int;
type E is uint;

function suffix(M x, E y) pure suffix returns (M) {}
// ----
// TypeError 1587: (47-57): Literal suffix function has invalid parameter types. The mantissa parameter must be an integer. The exponent parameter must be an unsigned integer.
