contract C {
    function f(uint[] x) public pure {
    }
}
// ====
// stopAfter: analysis
// ----
// TypeError 6651: (28-36): Data location must be "memory" or "calldata" for parameter in function, but none was given.
