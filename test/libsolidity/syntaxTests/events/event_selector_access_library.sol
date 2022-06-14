library L {
    event E();
}

contract C {
    function f() external pure returns (bytes32) {
        return L.E.selector;
    }
}
// ----
