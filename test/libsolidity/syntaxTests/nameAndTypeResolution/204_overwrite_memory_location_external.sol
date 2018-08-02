contract C {
    function f(uint[] memory a) external {}
}
// ----
// TypeError: (28-43): Storage location must be one of none, "calldata" for parameter in external function, but "memory" was given.
