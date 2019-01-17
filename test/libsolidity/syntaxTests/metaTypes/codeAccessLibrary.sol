contract Test {
    function creationOther() public pure returns (bytes memory) {
        return type(Library).creationCode;
    }
    function runtime() public pure returns (bytes memory) {
        return type(Library).runtimeCode;
    }
}
contract Library {
    function f(uint) public pure returns (uint) {}
}
// ----
