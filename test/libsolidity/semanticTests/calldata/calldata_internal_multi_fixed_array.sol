contract C {
    function g(uint[3][2] calldata s) internal pure returns (uint, uint[3] calldata) {
        return (s[0][1], s[1]);
    }
    function f(uint, uint[3][2] calldata s, uint) external pure returns (uint, uint) {
        (uint x, uint[3] calldata y) = g(s);
        return (x, y[0]);
    }
    function g() public returns (uint, uint) {
        uint[3][2] memory x;
        x[0][1] = 7;
        x[1][0] = 8;
        return this.f(4, x, 5);
    }
}
// ====
// compileViaYul: also
// ----
// g() -> 7, 8
