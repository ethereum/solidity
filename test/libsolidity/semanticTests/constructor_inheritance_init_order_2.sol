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
// gas irOptimized: 99436
// gas irOptimized code: 20200
// gas legacy: 100973
// gas legacy code: 32600
// gas legacyOptimized: 99137
// gas legacyOptimized code: 16200
// y() -> 42
