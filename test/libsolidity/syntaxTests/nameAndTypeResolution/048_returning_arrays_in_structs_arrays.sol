contract C {
    struct S { string[] s; }
    function f() public pure returns (S x) {}
}
// ----
// TypeError: (80-83): This type is only supported in the new experimental ABI encoder. Use "pragma experimental ABIEncoderV2;" to enable the feature.
