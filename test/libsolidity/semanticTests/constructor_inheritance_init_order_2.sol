contract A {
    uint x = 42;
    function f() public returns(uint256) {
        return x;
    }
}
contract B is A {
    uint public y = f();
}
// ----
// constructor() ->
// gas irOptimized: 119636
// gas legacy: 133574
// gas legacyOptimized: 115337
// y() -> 42
