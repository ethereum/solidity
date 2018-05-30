contract test {
    function f(bytes storage) external;
}
// ----
// TypeError: (31-36): Location has to be calldata for external functions (remove the "memory" or "storage" keyword).
