contract C {
    function f() public pure returns (uint[][2] memory) {}
}
// ----
// TypeError 4957: (51-67): This type is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.
