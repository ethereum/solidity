error E(uint a, uint b);
contract C {
    uint public x;
    function f(bool c) public {
        require(c, E(g(), 7));
    }
    function g() public returns (uint) {
        x++;
        return 2;
    }
}
// ====
// compileViaYul: also
// ----
// x() -> 0
// f(bool): true ->
// x() -> 1
// f(bool): false -> FAILURE, hex"85208890", hex"0000000000000000000000000000000000000000000000000000000000000002", hex"0000000000000000000000000000000000000000000000000000000000000007"
// x() -> 1
