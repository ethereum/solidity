using {add as +, unsub as -} for uint global;

function add(uint x, uint y) pure suffix returns (uint) {
    return x + y;
}

function unsub(uint x) pure suffix returns (uint) {
    return uint(-int(x));
}
// ----
// TypeError 8841: (0-45): Can only use "global" with user-defined types.
// TypeError 5332: (7-10): Operators can only be implemented for user-defined value types.
// TypeError 5332: (17-22): Operators can only be implemented for user-defined value types.
