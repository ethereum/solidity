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
// Warning: (74-143): Function state mutability can be restricted to pure
// Warning: (180-249): Function state mutability can be restricted to pure
// Warning: (281-350): Function state mutability can be restricted to pure
// Warning: (387-456): Function state mutability can be restricted to pure
