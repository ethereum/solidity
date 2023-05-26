interface I {
    event E();
}

contract C {
    function f() external pure returns (bytes32) {
        return I.E.selector;
    }
}
// ----
