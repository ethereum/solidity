contract C {
    function f() pure public {
        uint x;
        (x, ) = (1E111, 1);
    }
}
// ----
// TypeError 3390: (77-82): Invalid rational number.
