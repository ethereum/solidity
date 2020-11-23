contract test {
    function a(uint a, uint b, uint c) public returns (uint r) { r = a * 100 + b * 10 + c * 1; }
    function b() public returns (uint r) { r = a({a: 1, b: 2, c: 3}); }
    function c() public returns (uint r) { r = a({b: 2, c: 3, a: 1}); }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// b() -> 123
// c() -> 123
