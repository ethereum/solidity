contract C {
    function f(bool c) public {
        (c ? uint[2] : uint[2])[3];
    }
}
// ----
// TypeError 9717: (58-65): Invalid mobile type in true expression.
// TypeError 3703: (68-75): Invalid mobile type in false expression.
