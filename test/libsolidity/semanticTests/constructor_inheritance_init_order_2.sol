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
// gas irOptimized: 99452
// gas irOptimized code: 20400
// gas legacy: 100974
// gas legacy code: 32600
// gas legacyOptimized: 99137
// gas legacyOptimized code: 16200
// y() -> 42
