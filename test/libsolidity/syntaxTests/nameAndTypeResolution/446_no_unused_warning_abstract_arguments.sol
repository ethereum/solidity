contract C {
    function f(uint a) pure public returns (uint b);
}
// ----
// TypeError: (0-67): Contract "C" should be marked as abstract.
