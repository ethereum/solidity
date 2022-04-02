contract c {
    uint constant a = 1;
    function f() public { a = 2; }
}
// ----
// TypeError 6520: (64-65='a'): Cannot assign to a constant variable.
