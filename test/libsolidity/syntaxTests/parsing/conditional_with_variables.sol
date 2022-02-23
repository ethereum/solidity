contract A {
    function f() public {
        uint x = 3;
        uint y = 1;
        uint z = (x > y) ? x : y;
        uint w = x > y ? x : y;
    }
}
// ----
// Warning 2072: (87-93): Unused local variable.
// Warning 2072: (121-127): Unused local variable.
// Warning 2018: (17-150): Function state mutability can be restricted to pure
