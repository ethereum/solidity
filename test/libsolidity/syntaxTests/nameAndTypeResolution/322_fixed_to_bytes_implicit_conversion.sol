contract test {
    function f() public {
        fixed a = 3.25;
        bytes32 c = a; c;
    }
}
// ----
// TypeError 9574: (74-87='bytes32 c = a'): Type fixed128x18 is not implicitly convertible to expected type bytes32.
