contract C {
    function g(int x, int y) public pure returns (int) { return x - y; }
    function h(int y, int x) public pure returns (int) { return y - x; }

    function f() public pure returns (int) {
        return (false ? g : h)(2, 1);
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 1
