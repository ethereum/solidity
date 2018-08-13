contract C {
    function f(function() external returns (mapping(uint=>uint) storage)) public pure {
    }
}
// ----
// TypeError: (57-76): Internal type cannot be used for external function type.
