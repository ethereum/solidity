contract C {
    uint constant x = a;
    uint constant a = b * c;
    uint constant b = c;
    uint constant c = b;
}
// ----
// TypeError: (17-36): The value of the constant x has a cyclic dependency via a.
// TypeError: (42-65): The value of the constant a has a cyclic dependency via b.
// TypeError: (71-90): The value of the constant b has a cyclic dependency via c.
// TypeError: (96-115): The value of the constant c has a cyclic dependency via b.
