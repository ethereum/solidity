library D { function double(bytes32 self) public returns (uint) { return 2; } }
contract C {
    using D for *;
    function f(uint a) public returns (uint) {
        // Bound to a, but self type does not match.
        return a.double();
    }
}
// ----
// TypeError: (227-235): Member "double" not found or not visible after argument-dependent lookup in uint256.
