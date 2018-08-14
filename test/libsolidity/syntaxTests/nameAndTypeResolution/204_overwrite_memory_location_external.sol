contract C {
    function f(uint[] memory a) external {}
}
// ----
// TypeError: (28-43): Data location must be "calldata" for parameter in external function, but "memory" was given.
