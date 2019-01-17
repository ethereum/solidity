contract C {
    function f(bytes20 x) public pure returns (address payable) {
        return address(x);
    }
}
