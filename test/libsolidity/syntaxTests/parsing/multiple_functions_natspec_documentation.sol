contract test {
    uint256 stateVar;
    /// This is test function 1
    function functionName1(bytes32 input) public returns (bytes32 out) {}
    /// This is test function 2
    function functionName2(bytes32 input) public returns (bytes32 out) {}
    // nothing to see here
    function functionName3(bytes32 input) public returns (bytes32 out) {}
    /// This is test function 4
    function functionName4(bytes32 input) public returns (bytes32 out) {}
}
// ----
// Warning: (97-110): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (128-139): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (203-216): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (234-245): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (304-317): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (335-346): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (410-423): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (441-452): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (74-143): Function state mutability can be restricted to pure
// Warning: (180-249): Function state mutability can be restricted to pure
// Warning: (281-350): Function state mutability can be restricted to pure
// Warning: (387-456): Function state mutability can be restricted to pure
