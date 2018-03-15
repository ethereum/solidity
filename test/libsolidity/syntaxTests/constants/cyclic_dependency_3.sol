contract C {
    uint constant x = a;
    uint constant a = b * c;
    uint constant b = c;
    uint constant c = b;
}
// ----
// TypeError: The value of the constant x has a cyclic dependency via a.
// TypeError: The value of the constant a has a cyclic dependency via b.
// TypeError: The value of the constant b has a cyclic dependency via c.
// TypeError: The value of the constant c has a cyclic dependency via b.
