contract test {
    function f(bytes storage) external;
}
// ----
// TypeError: (31-44): Data location must be "calldata" for parameter in external function, but "storage" was given.
