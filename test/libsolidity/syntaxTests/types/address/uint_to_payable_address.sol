contract C {
    function f(uint x) public pure returns (address payable) {
        return address(uint160(x));
    }
}
