contract C {
    uint constant a = a;
}
// ----
// TypeError 6161: (17-36='uint constant a = a'): The value of the constant a has a cyclic dependency via a.
