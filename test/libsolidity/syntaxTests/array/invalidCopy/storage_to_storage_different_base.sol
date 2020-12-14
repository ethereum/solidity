contract C {
    bytes1[32] data1;
    bytes2[10] data2;
    function f() external {
        data1 = data2;
    }
}
// ----
// TypeError 7407: (101-106): Type bytes2[10] storage ref is not implicitly convertible to expected type bytes1[32] storage ref.
