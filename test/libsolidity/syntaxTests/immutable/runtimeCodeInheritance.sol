contract Base {
    address public immutable user = address(0x0);
}

contract Derived is Base {}

contract Test {
    function test() public pure returns(bytes memory) {
        return type(Derived).runtimeCode;
    }
}
// ----
// TypeError 9274: (185-210): "runtimeCode" is not available for contracts containing immutable variables.
