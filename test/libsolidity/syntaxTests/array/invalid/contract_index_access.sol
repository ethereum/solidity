contract C {
    function f() view public {
        C[0];
    }
}
// ----
// TypeError: (52-56): Index access for contracts or libraries is not possible.
