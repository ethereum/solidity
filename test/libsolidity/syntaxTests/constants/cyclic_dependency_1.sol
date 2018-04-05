contract C {
    uint constant a = a;
}
// ----
// TypeError: The value of the constant a has a cyclic dependency via a.
