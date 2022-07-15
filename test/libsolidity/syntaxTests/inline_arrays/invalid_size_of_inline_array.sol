contract C {
    function f() public {
        uint[3] memory x = [1, 2];
    }
}
// ----
// TypeError 9574: (47-72): Type inline_array(int_const 1, int_const 2) is not implicitly convertible to expected type uint256[3] memory. Number of components in array literal (2) does not match array size (3).
