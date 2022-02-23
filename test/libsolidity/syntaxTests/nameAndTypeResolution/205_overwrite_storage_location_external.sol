contract C {
    function f(uint[] storage a) external {}
}
// ----
// TypeError 6651: (28-44): Data location must be "memory" or "calldata" for parameter in external function, but "storage" was given.
