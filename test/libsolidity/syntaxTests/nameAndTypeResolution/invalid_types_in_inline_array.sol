contract C {
    function f() public {
        uint[3] x = [45, 'foo', true];
    }
}
// ----
// Warning: (47-56): Variable is declared as a storage pointer. Use an explicit "storage" keyword to silence this warning.
// TypeError: (59-76): Unable to deduce common type for array elements.
