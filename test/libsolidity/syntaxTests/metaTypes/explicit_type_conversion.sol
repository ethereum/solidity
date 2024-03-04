contract C {
    function f(bool c) pure public returns (int) {
        return (c ? int : int)(0);
    }
}
// ----
// TypeError 9717: (84-87): Invalid mobile type in true expression.
// TypeError 3703: (90-93): Invalid mobile type in false expression.
