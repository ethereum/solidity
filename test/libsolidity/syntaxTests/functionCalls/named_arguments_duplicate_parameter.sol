contract test {
    function a(uint a, uint b) public returns (uint r) {
        r = a + b;
    }
    function b() public returns (uint r) {
        r = a({a: 1, a: 2});
    }
}
// ----
// Warning 2519: (31-37='uint a'): This declaration shadows an existing declaration.
// Warning 2519: (39-45='uint b'): This declaration shadows an existing declaration.
// TypeError 6995: (159-160='1'): Duplicate named argument "a".
