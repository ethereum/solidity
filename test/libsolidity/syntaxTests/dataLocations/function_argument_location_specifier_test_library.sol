library test {
    function f(bytes calldata) public;
}
// ----
// TypeError: (30-35): Location cannot be calldata for non-external functions (remove the "calldata" keyword).
