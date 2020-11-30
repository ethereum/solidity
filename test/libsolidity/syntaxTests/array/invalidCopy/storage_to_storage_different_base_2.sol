contract C {
    uint64[32] data1;
    uint256[10] data2;
    function f() external {
        data1 = data2;
    }
}
// ----
// TypeError 7407: (102-107): Type uint256[10] storage ref is not implicitly convertible to expected type uint64[32] storage ref.
