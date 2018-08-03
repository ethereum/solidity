contract test {
    function f(bytes memory) external;
}
// ----
// TypeError: (31-36): Storage location must be none or "calldata" for parameter in external function, but "memory" was given.
