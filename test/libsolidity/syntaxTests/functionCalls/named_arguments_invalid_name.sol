contract test {
    function a(uint a, uint b) public returns (uint r) {
        r = a + b;
    }
    function b() public returns (uint r) {
        r = a({a: 1, c: 2});
    }
}
// ----
// Warning 2519: (31-37): This declaration shadows an existing declaration.
// TypeError 4974: (153-168): Named argument "c" does not match function declaration.
