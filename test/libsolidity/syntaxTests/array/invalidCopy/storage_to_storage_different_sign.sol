contract C {
    int256[32] data1;
    uint256[10] data2;
    function f() external {
        data1 = data2;
    }
    function g() external {
        data2 = data1;
    }
}
// ----
// TypeError 7407: (102-107): Type uint256[10] storage ref is not implicitly convertible to expected type int256[32] storage ref.
// TypeError 7407: (159-164): Type int256[32] storage ref is not implicitly convertible to expected type uint256[10] storage ref.
