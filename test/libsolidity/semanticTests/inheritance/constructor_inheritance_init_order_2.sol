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
// gas irOptimized: 99420
// gas irOptimized code: 20000
// gas legacy: 100973
// gas legacy code: 32600
// gas legacyOptimized: 99137
// gas legacyOptimized code: 16200
// gas ssaCFGOptimized: 99321
// gas ssaCFGOptimized code: 18600
// y() -> 42
