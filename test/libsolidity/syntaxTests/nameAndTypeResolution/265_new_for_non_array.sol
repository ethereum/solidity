contract C {
    function f(uint size) public {
        uint x = new uint(7);
    }
}
// ----
// TypeError: (65-73): Contract or array type expected.
