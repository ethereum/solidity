error E(uint a, uint b);
contract C {
    function f() public pure {
        revert(E(2, 7));
    }
}
// ----
// TypeError 9322: (77-83): No matching declaration found after argument-dependent lookup.
