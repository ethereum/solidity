contract test {
    function a(uint a, uint b, uint c) public returns (uint r) { r = a * 100 + b * 10 + c * 1; }
    function b() public returns (uint r) { r = a({c: 3, a: 1, b: 2}); }
}
// ====
// compileViaYul: also
// ----
// b() -> 123
