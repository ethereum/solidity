error E(uint);
function f(E x) pure returns (uint) {}
// ----
// TypeError 5172: (26-27): Name has to refer to a user-defined value type, struct, enum or contract.
