contract test {
    function f(bytes calldata) internal;
}
// ----
// TypeError: (0-58): Contract "test" should be marked as abstract.
// TypeError: (20-56): Functions without implementation must be marked virtual.
