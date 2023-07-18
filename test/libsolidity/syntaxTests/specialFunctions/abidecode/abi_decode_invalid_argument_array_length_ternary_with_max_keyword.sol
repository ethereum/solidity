contract C {
    function f() pure public {
        bool x;
        abi.decode("", (uint[type(x = true ? uint8 : uint8).max]));
    }
}
// ----
// TypeError 9717: (105-110): Invalid mobile type in true expression.
// TypeError 3703: (113-118): Invalid mobile type in false expression.
