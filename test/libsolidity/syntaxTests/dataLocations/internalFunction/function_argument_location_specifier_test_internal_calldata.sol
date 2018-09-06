contract test {
    function f(bytes calldata) internal;
}
// ----
// TypeError: (31-45): Data location must be "storage" or "memory" for parameter in function, but "calldata" was given.
