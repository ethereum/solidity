contract C {
    function f(bytes memory b) public pure returns (address payable) {
        (address payable c) = abi.decode(b, (address));
        return c;
    }
}