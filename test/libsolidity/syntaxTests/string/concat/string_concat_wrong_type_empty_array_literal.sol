contract C {
    function f() public {
        string.concat([], [], []);
    }
}
// ----
// TypeError 6378: (61-63): Unable to deduce common type for array elements.
