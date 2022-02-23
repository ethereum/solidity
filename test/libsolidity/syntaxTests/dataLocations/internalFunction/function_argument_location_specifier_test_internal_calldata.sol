contract test {
    function f(bytes calldata) internal;
}
// ----
// TypeError 3656: (0-58): Contract "test" should be marked as abstract.
// TypeError 5424: (20-56): Functions without implementation must be marked virtual.
