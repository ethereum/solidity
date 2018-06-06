contract C {
    function f() pure public {
        uint x;
        (x, ) = (1, 1E111);
    }
}
// ----
// TypeError: (80-85): Invalid rational number.
