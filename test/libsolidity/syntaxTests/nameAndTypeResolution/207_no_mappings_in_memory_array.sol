contract C {
    function f() public {
        mapping(uint=>uint)[] memory x;
    }
}
// ----
// TypeError: (47-77): Data location must be "storage" for variable, but "memory" was given.
