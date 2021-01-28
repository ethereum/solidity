error E(uint a, uint b);
contract C {
    function f(bool c) public pure {
        require(c, E({b: 7, a: 2}));
    }
}
// ====
// compileViaYul: also
// ----
// f(bool): true ->
// f(bool): false -> FAILURE, hex"85208890", hex"0000000000000000000000000000000000000000000000000000000000000002", hex"0000000000000000000000000000000000000000000000000000000000000007"
