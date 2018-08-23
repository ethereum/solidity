contract test {
    function f(bytes storage) external;
}
// ----
// TypeError: (31-36): Data location must be "calldata" for parameter in external function, but "storage" was given.
