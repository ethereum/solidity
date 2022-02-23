// Used to cause ICE
contract C {
    function f(uint[] calldata x) external pure {
        x[1:2].a;
    }
}
// ----
// TypeError 9582: (92-100): Member "a" not found or not visible after argument-dependent lookup in uint256[] calldata slice.
