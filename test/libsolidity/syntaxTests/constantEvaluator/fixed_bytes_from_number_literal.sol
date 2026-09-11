contract C {
    // TypeChecker has not yet run when `a` is evaluated in the array length
    bytes32 constant x = 1;
    uint[x & 1] y;
}
// ----
// TypeError 6020: (127-132): Operator & not compatible with types bytes32 and int_const 1
