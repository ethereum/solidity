pragma abicoder v1;
contract C {
    function f() public pure returns (string[][] memory) {}
}
// ----
// TypeError 4957: (71-88): This type is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.
