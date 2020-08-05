contract test {
    function f(bytes storage) external;
}
// ----
// TypeError 6651: (31-44): Data location must be "memory" or "calldata" for parameter in external function, but "storage" was given.
