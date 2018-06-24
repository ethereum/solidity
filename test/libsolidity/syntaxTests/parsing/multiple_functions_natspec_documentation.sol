contract test {
    uint256 stateVar;
    /// This is test function 1
    function functionName1(bytes32 input) returns (bytes32 out) {}
    /// This is test function 2
    function functionName2(bytes32 input) returns (bytes32 out) {}
    // nothing to see here
    function functionName3(bytes32 input) returns (bytes32 out) {}
    /// This is test function 4
    function functionName4(bytes32 input) returns (bytes32 out) {}
}
// ----
// Warning: (74-136): No visibility specified. Defaulting to "public". 
// Warning: (97-110): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (121-132): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (173-235): No visibility specified. Defaulting to "public". 
// Warning: (196-209): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (220-231): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (267-329): No visibility specified. Defaulting to "public". 
// Warning: (290-303): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (314-325): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (366-428): No visibility specified. Defaulting to "public". 
// Warning: (389-402): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (413-424): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (74-136): Function state mutability can be restricted to pure
// Warning: (173-235): Function state mutability can be restricted to pure
// Warning: (267-329): Function state mutability can be restricted to pure
// Warning: (366-428): Function state mutability can be restricted to pure
