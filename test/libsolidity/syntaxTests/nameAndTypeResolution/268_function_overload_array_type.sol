contract M {
    function f(uint[] memory) public;
    function f(int[] memory) public;
}
// ----
// TypeError: (0-89): Contract "M" should be marked as abstract.
