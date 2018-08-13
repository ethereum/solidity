contract C {
    function f(mapping(uint => uint)[] storage) external pure {
    }
}
// ----
// TypeError: (28-51): Location has to be calldata for external functions (remove the "memory" or "storage" keyword).
