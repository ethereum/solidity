contract C {
    function f() public pure returns (uint, uint, uint) {
        return (1, 2, 3);
    }
    function g() public pure returns (uint a, uint b, uint c) {
        (c, b, a) = f();
    }
    function h() public pure returns (uint a) {
        (,,a) = f();
    }
}
// ----
// g() -> 3, 2, 1
// h() -> 3
