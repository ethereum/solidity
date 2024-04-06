contract test {
    function f(bytes transient) external;
}
// ----
// TypeError 6651: (31-46): Data location must be "memory" or "calldata" for parameter in external function, but none was given.
