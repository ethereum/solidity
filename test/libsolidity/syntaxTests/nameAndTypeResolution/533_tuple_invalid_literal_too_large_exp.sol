contract C {
    function f() pure public {
        (2**270, 1);
    }
}
// ----
// TypeError: (53-59): Invalid rational number.
