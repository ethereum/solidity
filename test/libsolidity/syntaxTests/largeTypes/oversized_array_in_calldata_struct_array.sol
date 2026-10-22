contract C {
    struct S { uint256[][4294967292] x; }
    function foo() public returns (S[] calldata) { foo(); }
}
// ----
// TypeError 1534: (90-102): Type too large for calldata.
