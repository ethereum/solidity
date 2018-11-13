contract test {
    function f() public {
        fixed[3] memory a = [fixed(3.5), fixed(-4.25), fixed(967.125)];
    }
}
// ----
// Warning: (50-67): Unused local variable.
// Warning: (20-119): Function state mutability can be restricted to pure
