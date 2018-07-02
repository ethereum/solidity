contract A {
    function f() public {
        uint x = 3 > 0 ? 3 : 0;
        uint y = (3 > 0) ? 3 : 0;
    }
}
// ----
// Warning: (47-53): Unused local variable.
// Warning: (79-85): Unused local variable.
// Warning: (17-110): Function state mutability can be restricted to pure
