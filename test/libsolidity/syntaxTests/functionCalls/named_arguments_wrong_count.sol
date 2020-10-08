contract test {
    function a(uint a, uint b) public returns (uint r) {
        r = a + b;
    }
    function b() public returns (uint r) {
        r = a({a: 1});
    }
}
// ----
// Warning 2519: (31-37): This declaration shadows an existing declaration.
// Warning 2519: (39-45): This declaration shadows an existing declaration.
// TypeError 6160: (153-162): Wrong argument count for function call: 1 arguments given but expected 2.
