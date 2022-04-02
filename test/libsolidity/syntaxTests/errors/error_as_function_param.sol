error E(uint);
function f(E x) pure returns (uint) {}
// ----
// TypeError 5172: (26-27='E'): Name has to refer to a struct, enum or contract.
