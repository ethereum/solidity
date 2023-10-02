event E();

contract C {
    function f() external pure returns (bytes32) {
        return E.selector;
    }
}
