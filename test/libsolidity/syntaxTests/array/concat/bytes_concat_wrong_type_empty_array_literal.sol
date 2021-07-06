contract C {
    function f() public {
        bytes.concat([], [], []);
    }
}
// ----
// TypeError 6378: (60-62): Unable to deduce common type for array elements.
