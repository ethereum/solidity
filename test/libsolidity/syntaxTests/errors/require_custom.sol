error E(uint a, uint b);
contract C {
    function f(bool c) public pure {
        require(c, E(2, 7));
    }
}
// ----
// TypeError 9322: (83-90='require'): No matching declaration found after argument-dependent lookup.
