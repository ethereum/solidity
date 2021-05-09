pragma abicoder v1;
contract C {
    struct S { string[] s; }
    function f() public pure returns (S memory x) {}
}
// ----
// TypeError 4957: (100-110): This type is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.
