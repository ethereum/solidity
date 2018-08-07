contract test {
    function f(bytes calldata) public;
}
// ----
// TypeError: (31-36): Data location must be "memory" for parameter in function, but "calldata" was given.
