contract C {
    function f() public pure returns (uint, uint, uint) {
        return (1, 2, 3);
    }
    function g() public pure returns (uint x, uint y, uint z) {
        (uint c, uint b, uint a) = f();
        (x, y, z) = (a, b, c);
    }
    function h() public pure returns (uint) {
        (,,uint a) = f();
        return a;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// g() -> 3, 2, 1
// h() -> 3
