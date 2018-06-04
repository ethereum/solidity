contract C {
    function f() public view returns (bytes32) { return blockhash(3); }
}
