library D { function double(uint self) public returns (uint) { return 2; } }
contract C {
    using D for uint;
    function f(uint16 a) public returns (uint) {
        // This is an error because the function is only attached to uint.
        // Had it been attached to *, it would have worked.
        return a.double();
    }
}
// ----
// TypeError 9582: (311-319): Member "double" not found or not visible after argument-dependent lookup in uint16.
