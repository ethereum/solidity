contract C {
    function f() pure public {
        int x;
        abi.decode("", ((x = 1) > 0 ? int : int));
    }
}
// ----
// TypeError 9717: (97-100): Invalid mobile type in true expression.
// TypeError 3703: (103-106): Invalid mobile type in false expression.
