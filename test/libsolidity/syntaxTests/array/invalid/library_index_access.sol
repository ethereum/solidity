library C {
    function f() view public {
        C[0];
    }
}
// ----
// TypeError: (51-55): Index access for contracts or libraries is not possible.
