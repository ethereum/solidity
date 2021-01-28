error error(uint a);
contract C {
    function f() public pure {
        revert(error(2));
    }
}
// ====
// compileViaYul: also
// ----
// f() -> FAILURE, hex"85208890", hex"0000000000000000000000000000000000000000000000000000000000000002", hex"0000000000000000000000000000000000000000000000000000000000000007"
