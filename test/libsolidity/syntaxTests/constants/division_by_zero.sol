contract c {
    uint constant a1 = 0;
    uint constant a2 = 1;
    uint constant b1 = 7 / a1;
    uint constant b2 = 7 / (a2 - 1);
}
// ----
// TypeError: (88-94): Division by zero.
// TypeError: (119-131): Division by zero.
