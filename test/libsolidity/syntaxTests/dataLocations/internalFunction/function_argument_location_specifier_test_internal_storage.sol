contract test {
    function f(bytes storage) internal;
}
// ----
// TypeError: (0-57): Contract "test" should be marked as abstract.
