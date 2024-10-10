contract test {
    function f(bytes transient) public;
}
// ----
// TypeError 6651: (31-46): Data location must be "memory" or "calldata" for parameter in function, but none was given.
