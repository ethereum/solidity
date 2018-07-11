contract C {
    function f() public {
        uint[3] memory x = [45, 'foo', true];
    }
}
// ----
// TypeError: (66-83): Unable to deduce common type for array elements.
