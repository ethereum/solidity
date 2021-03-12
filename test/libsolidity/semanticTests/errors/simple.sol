error E(uint a, uint b);
contract C {
    function f() public pure {
        revert E(2, 7);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> FAILURE, hex"85208890", hex"0000000000000000000000000000000000000000000000000000000000000002", hex"0000000000000000000000000000000000000000000000000000000000000007"
