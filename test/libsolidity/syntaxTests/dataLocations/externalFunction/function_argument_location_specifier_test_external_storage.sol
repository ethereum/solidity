contract test {
    function f(bytes storage) external;
}
// ----
// TypeError: (31-36): Storage location must be none or "calldata" for parameter in external function, but "storage" was given.
