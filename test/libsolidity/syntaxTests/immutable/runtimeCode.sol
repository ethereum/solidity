contract A {
    address public immutable user = address(0x0);
}

contract Test {
    function test() public pure returns(bytes memory) {
        return type(A).runtimeCode;
    }
}
// ----
// TypeError 9274: (153-172): "runtimeCode" is not available for contracts containing immutable variables.
