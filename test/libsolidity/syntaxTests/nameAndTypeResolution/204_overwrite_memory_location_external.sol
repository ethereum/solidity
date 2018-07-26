contract C {
    function f(uint[] memory a) external {}
}
// ----
// TypeError: (28-43): Location has to be calldata for external function. Remove the data location keyword to fix this error.
