contract Test {
    function creationOther() public pure returns (bytes memory) {
        return type(Other).creationCode;
    }
    function runtimeOther() public pure returns (bytes memory) {
        return type(Other).runtimeCode;
    }
}
contract Other {
    function f(uint) public pure returns (uint) {}
}
// ----
