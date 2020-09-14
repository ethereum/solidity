contract C {
    event e(uint u, string s);
    event e(string s, uint u);

    function call() public {
        emit e({s: 2, u: "abc"});
    }
}
// ----
// TypeError 9322: (118-119): No matching declaration found after argument-dependent lookup.
