library test {
    function f(bytes memory) external;
}
// ----
// TypeError: (30-42): Data location must be "storage" or "calldata" for parameter in external function, but "memory" was given.
