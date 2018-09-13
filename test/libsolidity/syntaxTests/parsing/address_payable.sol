contract C {
    address payable a;
    function f(address payable b) public pure returns (address payable c) {
        address payable d = b;
        return d;
    }
}
