contract C {
    uint constant x = a;
    uint constant a = b * c;
    uint constant b = c;
    uint constant c = b;
}
// ----
// TypeError 5210: (89-90): Cyclic constant definition (or maximum recursion depth exhausted).
