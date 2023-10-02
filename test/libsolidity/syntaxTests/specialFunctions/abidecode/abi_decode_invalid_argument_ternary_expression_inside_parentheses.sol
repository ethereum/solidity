contract C {
    function f() pure public {
        abi.decode("", ((true ? uint : uint)));
    }
}
// ----
// TypeError 9717: (76-80): Invalid mobile type in true expression.
// TypeError 3703: (83-87): Invalid mobile type in false expression.
