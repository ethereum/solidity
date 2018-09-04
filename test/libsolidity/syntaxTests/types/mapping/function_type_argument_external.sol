contract C {
    function f(function(mapping(uint=>uint) storage) external) public pure {
    }
}
// ----
// TypeError: (37-64): Data location must be "memory" for parameter in function, but "storage" was given.
// TypeError: (37-64): Internal type cannot be used for external function type.
