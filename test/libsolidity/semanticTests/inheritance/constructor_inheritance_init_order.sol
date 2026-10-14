contract A {
    uint x;
    constructor() {
        x = 42;
    }
    function f() public returns(uint256) {
        return x;
    }
}
contract B is A {
    uint public y = f();
}
// ====
// compileViaYul: true
// ----
// constructor() ->
// gas irOptimized: 99420
// gas irOptimized code: 20000
// gas ssaCFGOptimized: 99321
// gas ssaCFGOptimized code: 18600
// y() -> 42
