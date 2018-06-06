contract C {
    enum small { A, B, C, D }
    enum big { A, B, C, D }

    function f() public {
        small x;
        big y;

        true ? x : y;
    }
}
// ----
// TypeError: (139-151): True expression's type enum C.small doesn't match false expression's type enum C.big.
