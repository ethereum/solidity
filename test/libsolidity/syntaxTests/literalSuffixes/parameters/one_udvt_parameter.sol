type B is bool;

function suffix(B x) pure suffix returns (B) {}
// ----
// TypeError 2998: (33-36): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
