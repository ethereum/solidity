contract A {
    uint public x = 0;
    uint y = f();
    function f() public returns (uint256) {
        ++x;
        return 42;
    }
}
contract B is A {
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// x() -> 1
