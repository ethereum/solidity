contract test {
    function f(bytes calldata) internal;
}
// ----
// TypeError: (31-36): Storage location must be one of: "storage", "memory" for parameter in internal function, but "calldata" was given.
