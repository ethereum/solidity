contract C {
    function f(function(mapping(uint=>uint) storage) external) public pure {
    }
}
// ----
// TypeError: (37-64): Data location must be "memory" for parameter in function, but "storage" was given.
