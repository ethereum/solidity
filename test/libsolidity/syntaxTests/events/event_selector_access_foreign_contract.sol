contract D {
    event E();
}

contract C {
    function f() external pure returns (bytes32) {
        return D.E.selector;
    }
}
// ----
