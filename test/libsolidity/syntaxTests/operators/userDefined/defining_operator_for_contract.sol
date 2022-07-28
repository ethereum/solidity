using {fc as +} for C;
using {fa as +} for A;

function fc(C, C) returns (C) {}
function fa(A, A) returns (A) {}

contract C {}
abstract contract A {}
// ----
// TypeError 5332: (7-9): Operators can only be implemented for user-defined value types.
// TypeError 5332: (30-32): Operators can only be implemented for user-defined value types.
