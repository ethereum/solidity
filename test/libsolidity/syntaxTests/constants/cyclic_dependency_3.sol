contract C {
    uint constant x = a;
    uint constant a = b * c;
    uint constant b = c;
    uint constant c = b;
}
// ----
// TypeError 6161: (17-36='uint constant x = a'): The value of the constant x has a cyclic dependency via a.
// TypeError 6161: (42-65='uint constant a = b * c'): The value of the constant a has a cyclic dependency via b.
// TypeError 6161: (71-90='uint constant b = c'): The value of the constant b has a cyclic dependency via c.
// TypeError 6161: (96-115='uint constant c = b'): The value of the constant c has a cyclic dependency via b.
