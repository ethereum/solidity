contract C {
    function f(uint[] memory a) external {}
}
// ----
// TypeError: (28-43): Storage location must be none or "calldata" for parameter in external function, but "memory" was given.
