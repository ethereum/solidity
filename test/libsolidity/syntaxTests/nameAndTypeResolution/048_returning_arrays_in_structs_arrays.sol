contract C {
    struct S { string[] s; }
    function f() public pure returns (S memory x) {}
}
// ----
// TypeError 4957: (80-90): This type is only supported in ABIEncoderV2. Use "pragma experimental ABIEncoderV2;" to enable the feature.
