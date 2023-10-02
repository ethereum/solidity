contract C {
    function f() pure public {
        bool x;
        abi.decode("", ((x = true ? uint : uint)[0]));
    }
}
// ----
// TypeError 9717: (96-100): Invalid mobile type in true expression.
// TypeError 3703: (103-107): Invalid mobile type in false expression.
