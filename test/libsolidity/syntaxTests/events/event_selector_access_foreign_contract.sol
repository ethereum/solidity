contract D {
    event E();
}

contract C {
    function f() external pure returns (bytes32) {
        return D.E.selector;
    }
}
// ----
// TypeError 9582: (110-113): Member "E" not found or not visible after argument-dependent lookup in type(contract D).
