contract test {
    function f(bytes calldata) public;
}
// ----
// TypeError: (0-56): Contract "test" should be marked as abstract.
// TypeError: (20-54): Functions without implementation must be marked virtual.
