contract C {
    function f(uint[] memory a) external {}
}
// ----
// TypeError: (28-43): Location has to be calldata for external functions (remove the "memory" or "storage" keyword).
