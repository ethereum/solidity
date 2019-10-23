contract test {
    function f(bytes calldata) external;
}
// ----
// TypeError: (0-58): Contract "test" should be marked as abstract.
