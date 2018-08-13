contract C {
    function f(function(mapping(uint=>uint) storage) external) public pure {
    }
}
// ----
// TypeError: (37-56): Internal type cannot be used for external function type.
