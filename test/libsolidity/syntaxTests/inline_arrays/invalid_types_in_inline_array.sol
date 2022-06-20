contract C {
    function f() public {
        uint[3] memory x = [45, 'foo', true];
    }
}
// ----
// TypeError 6069: (71-76): Type literal_string "foo" is not implicitly convertible to expected type uint256.
// TypeError 6069: (78-82): Type bool is not implicitly convertible to expected type uint256.
// TypeError 6378: (66-83): Unable to deduce common type for array elements.
