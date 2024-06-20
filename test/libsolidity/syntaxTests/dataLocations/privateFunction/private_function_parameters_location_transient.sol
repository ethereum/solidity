contract C {
    function f(uint[] transient) private pure {}
}
// ----
// TypeError 6651: (28-44): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
