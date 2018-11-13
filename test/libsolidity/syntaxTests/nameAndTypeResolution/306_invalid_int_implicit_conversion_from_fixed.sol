contract test {
    function f() public {
        fixed a = 4.5;
        int b = a;
        a; b;
    }
}
// ----
// TypeError: (73-82): Type fixed128x18 is not implicitly convertible to expected type int256.
