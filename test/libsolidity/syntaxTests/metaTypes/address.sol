contract Test {
    function basic() public pure {
        address a = type(address).min;
        a;
        address b = type(address).max;
        b;
    }
}
