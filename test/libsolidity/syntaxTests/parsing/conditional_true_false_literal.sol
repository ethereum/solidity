contract A {
    function f() public {
        uint x = true ? 1 : 0;
        uint y = false ? 0 : 1;
    }
}
// ----
// Warning: (47-53): Unused local variable.
// Warning: (78-84): Unused local variable.
// Warning: (17-107): Function state mutability can be restricted to pure
