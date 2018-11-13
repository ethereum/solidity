contract test {
    function f(bytes memory) external;
}
// ----
// TypeError: (31-43): Data location must be "calldata" for parameter in external function, but "memory" was given.
