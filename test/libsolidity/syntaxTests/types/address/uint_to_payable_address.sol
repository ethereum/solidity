contract C {
    function f(uint x) public pure returns (address payable) {
        return payable(address(uint160(x)));
    }
}
