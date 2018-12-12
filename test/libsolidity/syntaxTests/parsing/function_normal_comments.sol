contract test {
    uint256 stateVar;
    // We won't see this comment
    function functionName(bytes32 input) public returns (bytes32 out) {}
}
// ----
// Warning: (75-143): Function state mutability can be restricted to pure
