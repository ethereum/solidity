contract test {
    uint256 stateVar;
    // We won't see this comment
    function functionName(bytes32 input) returns (bytes32 out) {}
}
// ----
// Warning: (75-136): No visibility specified. Defaulting to "public". 
// Warning: (97-110): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (121-132): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (75-136): Function state mutability can be restricted to pure
