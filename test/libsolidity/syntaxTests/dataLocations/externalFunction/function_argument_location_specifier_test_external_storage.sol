contract test {
    function f(bytes storage) external;
}
// ----
// TypeError: (31-36): Location has to be calldata for external function. Remove the data location keyword to fix this error.
