contract C {
    function f() public pure {
        bytes16 b;
        delete b[5];
    }
}
// ----
// TypeError 4360: (78-82): Single bytes in fixed bytes arrays cannot be modified.
