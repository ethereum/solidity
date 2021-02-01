contract A {
    uint x = 42;
    function f() public returns(uint256) {
        return x;
    }
}
contract B is A {
    uint public y = f();
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// constructor() ->
// gas ir: 232112
// gas irOptimized: 153851
// gas legacy: 151008
// gas legacyOptimized: 131422
// y() -> 42
