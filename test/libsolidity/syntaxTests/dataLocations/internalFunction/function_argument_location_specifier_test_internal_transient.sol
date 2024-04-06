contract test {
    function f(bytes transient) internal {}
}
// ----
// TypeError 6651: (31-46): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
