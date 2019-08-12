contract C {
    function blockhash(uint256 blockNumber) public returns(bytes32) { bytes32 x; return x; }
    function f() public returns(bytes32) { return blockhash(3); }
}
// ====
// compileViaYul: also
// ----
// f() -> 0
