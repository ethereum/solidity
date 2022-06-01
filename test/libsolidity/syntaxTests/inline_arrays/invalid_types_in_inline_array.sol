contract C {
    function f() public {
        uint[3] memory x = [45, 'foo', true];
    }
}
// ----
// TypeError 9574: (47-83): Type inline_array(int_const 45, literal_string "foo", bool) is not implicitly convertible to expected type uint256[3] memory. Invalid conversion from literal_string "foo" to uint256.
