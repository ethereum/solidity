using {fc as +} for C global;
using {fa as +} for A global;

function fc(C, C) pure returns (C) {}
function fa(A, A) pure returns (A) {}

contract C {}
abstract contract A {}
// ----
// TypeError 8841: (0-29): Can only use "global" with user-defined types.
// TypeError 5332: (7-9): Operators can only be implemented for user-defined value types.
// TypeError 8841: (30-59): Can only use "global" with user-defined types.
// TypeError 5332: (37-39): Operators can only be implemented for user-defined value types.
