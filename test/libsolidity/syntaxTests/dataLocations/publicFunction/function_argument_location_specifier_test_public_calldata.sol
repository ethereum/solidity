contract test {
    function f(bytes calldata) public;
}
// ----
// TypeError: (31-36): Location has to be memory for publicly visible functions (remove the "storage" or "calldata" keyword).
