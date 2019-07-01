contract test {
    function a(uint a, uint b, uint c) public returns (uint r) { r = a * 100 + b * 10 + c * 1; }
    function b() public returns (uint r) { r = a({a: 1, b: 2, c: 3}); }
}
// ----
// b() -> 123
