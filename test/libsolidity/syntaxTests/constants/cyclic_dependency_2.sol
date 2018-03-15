contract C {
    uint constant a = b * c;
    uint constant b = 7;
    uint constant c = b + uint(keccak256(d));
    uint constant d = 2 + a;
}
// ----
// TypeError: The value of the constant a has a cyclic dependency via c.
// TypeError: The value of the constant c has a cyclic dependency via d.
// TypeError: The value of the constant d has a cyclic dependency via a.
