contract test {
    function f() public {
        fixed a = 3.25;
        bytes32 c = a; c;
    }
}
// ----
// TypeError: (74-87): Type fixed128x18 is not implicitly convertible to expected type bytes32.
