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
// gas irOptimized: 119640
// gas legacy: 133594
// gas legacyOptimized: 115341
// y() -> 42
