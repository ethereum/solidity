contract C {
    function f() pure public {
        abi.decode("", ([uint][2]));
    }
}
// ----
// TypeError 9563: (69-73): Invalid mobile type.
