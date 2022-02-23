contract C {
    uint immutable x = 0;
    uint y = 0;

    function f() readX internal {
    }

    modifier readX() {
        _;
        y = x + 1;
    }
}
// ----
