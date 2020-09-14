contract C {
    event e(uint u, string s);
    event e(bytes s, int u);

    function call() public {
        emit e({u: 2, s: "abc"});
    }
}
// ----
// TypeError 4487: (116-117): No unique declaration found after argument-dependent lookup.
