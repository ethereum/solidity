contract test {
    function f(bytes memory) external;
}
// ----
// TypeError: (31-36): Location has to be calldata for external functions (remove the "memory" or "storage" keyword).
