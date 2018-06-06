contract C {
    function f(uint[] storage a) external {}
}
// ----
// TypeError: (28-44): Location has to be calldata for external functions (remove the "memory" or "storage" keyword).
