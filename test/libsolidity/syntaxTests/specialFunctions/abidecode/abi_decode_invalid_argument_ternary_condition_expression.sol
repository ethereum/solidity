contract C {
    function f() pure public {
        abi.decode("", (true ? uint : uint));
    }
}
// ----
// TypeError 9717: (75-79): Invalid mobile type in true expression.
// TypeError 3703: (82-86): Invalid mobile type in false expression.
