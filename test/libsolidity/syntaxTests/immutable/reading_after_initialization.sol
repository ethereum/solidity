contract C {
    uint immutable x = 0;
    uint y = 0;

    function f() internal {
        y = x + 1;
    }
}
// ----
