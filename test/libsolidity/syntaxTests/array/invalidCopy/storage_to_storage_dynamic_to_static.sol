contract C {
    uint256[10] data1;
    uint256[] data2;
    function f() external {
        data1 = data2;
    }
}
// ----
// TypeError 7407: (101-106): Type uint256[] storage ref is not implicitly convertible to expected type uint256[10] storage ref.
