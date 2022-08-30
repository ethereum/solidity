library A {
    uint256 constant VAL = B.VAL + 1;
}

library B {
    uint256 constant VAL = A.VAL + 1;
}

// ----
// TypeError 6161: (16-48): The value of the constant VAL has a cyclic dependency via VAL.
// TypeError 6161: (69-101): The value of the constant VAL has a cyclic dependency via VAL.
