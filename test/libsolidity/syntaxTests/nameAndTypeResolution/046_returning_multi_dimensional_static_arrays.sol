contract C {
    function f() public pure returns (uint[][2] memory) {}
}
// ----
// TypeError: (51-67): This type is only supported in the new experimental ABI encoder. Use "pragma experimental ABIEncoderV2;" to enable the feature.
