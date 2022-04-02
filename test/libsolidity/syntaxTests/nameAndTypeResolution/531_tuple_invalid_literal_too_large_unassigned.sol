contract C {
    function f() pure public {
        uint x;
        (x, ) = (1, 1E111);
    }
}
// ----
// TypeError 3390: (80-85='1E111'): Invalid rational number.
