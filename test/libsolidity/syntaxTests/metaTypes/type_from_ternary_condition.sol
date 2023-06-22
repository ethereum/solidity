contract C {
    function f(bool c) pure public {
        type(c ? uint : uint);
    }
}
// ----
// TypeError 9717: (67-71): Invalid mobile type in true expression.
// TypeError 3703: (74-78): Invalid mobile type in false expression.
