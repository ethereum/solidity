library test {
    function f(bytes transient) external {}
}
// ----
// TypeError 6651: (30-45): Data location must be "storage", "memory" or "calldata" for parameter in external function, but none was given.
