error E(uint a, uint b);
contract C {
    function f(bool c) public pure {
        require(c, E(2, 7));
    }
}
// ====
// compileViaYul: also
// ----
// f(bool): true ->
// f(bool): false -> FAILURE, hex"85208890", hex"0000000000000000000000000000000000000000000000000000000000000002", hex"0000000000000000000000000000000000000000000000000000000000000007"
