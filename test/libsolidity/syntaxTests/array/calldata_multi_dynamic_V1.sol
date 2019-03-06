contract Test {
    function f(uint[][] calldata) external { }
    function g(uint[][1] calldata) external { }
}
// ----
// TypeError: (31-48): This type is only supported in the new experimental ABI encoder. Use "pragma experimental ABIEncoderV2;" to enable the feature.
// TypeError: (78-96): This type is only supported in the new experimental ABI encoder. Use "pragma experimental ABIEncoderV2;" to enable the feature.
