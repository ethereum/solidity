contract C {
    function f() public pure returns (string[][] memory) {}
}
// ----
// TypeError: (51-61): This type is only supported in the new experimental ABI encoder. Use "pragma experimental ABIEncoderV2;" to enable the feature.
