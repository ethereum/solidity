contract test {
    function f(bytes memory) external;
}
// ----
// TypeError: (31-36): Storage location must be one of: none, "calldata" for parameter in external function, but "memory" was given.
