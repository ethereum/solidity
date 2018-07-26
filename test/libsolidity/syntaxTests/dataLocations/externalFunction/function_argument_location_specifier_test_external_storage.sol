contract test {
    function f(bytes storage) external;
}
// ----
// TypeError: (31-36): Storage location must be one of: none, "calldata" for parameter in external function, but "storage" was given.
