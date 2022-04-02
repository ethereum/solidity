contract C {
    function f() public {
        uint[3] memory x = [45, 'foo', true];
    }
}
// ----
// TypeError 6378: (66-83='[45, 'foo', true]'): Unable to deduce common type for array elements.
