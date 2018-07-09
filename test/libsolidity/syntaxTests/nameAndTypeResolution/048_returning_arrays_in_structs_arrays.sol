contract C {
    struct S { string[] s; }
    function f() public pure returns (S memory x) {}
}
// ----
// TypeError: (80-90): This type is only supported in the new experimental ABI encoder. Use "pragma experimental ABIEncoderV2;" to enable the feature.
