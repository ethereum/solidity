contract C {
    function f(uint[] storage a) external {}
}
// ----
// TypeError: (28-44): Location has to be calldata for external function. Remove the data location keyword to fix this error.
