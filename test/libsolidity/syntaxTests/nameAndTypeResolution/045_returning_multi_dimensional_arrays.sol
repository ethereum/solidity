contract C {
    function f() public pure returns (string[][] memory) {}
}
// ----
// TypeError: (51-68): This type is only supported in ABIEncoderV2. Use "pragma experimental ABIEncoderV2;" to enable the feature.
