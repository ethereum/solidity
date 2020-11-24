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
// compileViaYul: also
// compileToEwasm: also
// ----
// constructor() ->
// y() -> 42
