interface I {
    event E();
}

contract C {
    function f() external pure returns (bytes32) {
        return I.E.selector;
    }
}
// ----
// TypeError 9582: (111-114): Member "E" not found or not visible after argument-dependent lookup in type(contract I).
