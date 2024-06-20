library test {
    function f(bytes transient) internal pure {}
}
// ----
// TypeError 6651: (30-45): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
