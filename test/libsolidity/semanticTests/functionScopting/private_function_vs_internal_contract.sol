// Tests that private contract function is not inherited, but that internal function does.
contract A {
    function f() private pure returns (uint) { return 7; }
    function g() internal pure returns (uint) { return f(); }
}
contract B is A {
    function f() private pure returns (uint) { return 42; }
    function test() external pure returns (uint, uint) {
        return (g(), f());
  }
}
// ----
// test() -> 7, 0x2a
