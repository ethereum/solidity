contract C {
    address a;
    function f(address b) public pure returns (address c) {
        address d = b;
        return d;
    }
}
