contract A {
    address public immutable user = address(0x0);
}

contract Test {
    function test() public pure returns(bytes memory) {
        return type(A).creationCode;
    }
}
