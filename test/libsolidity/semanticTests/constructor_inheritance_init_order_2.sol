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
// gas irOptimized: 121153
// gas legacy: 135046
// gas legacyOptimized: 116176
// y() -> 42
