contract C {
    function f(uint size) public {
        var x = new uint[]();
    }
}
// ----
// Warning: (56-61): Use of the "var" keyword is deprecated.
// TypeError: (64-76): Wrong argument count for function call: 0 arguments given but expected 1.
