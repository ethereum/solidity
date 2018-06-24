contract C {
    function f(uint size) public {
        var x = new uint(7);
    }
}
// ----
// Warning: (56-61): Use of the "var" keyword is deprecated.
// TypeError: (64-72): Contract or array type expected.
