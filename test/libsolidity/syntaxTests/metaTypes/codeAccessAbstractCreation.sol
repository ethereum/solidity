contract Test {
    function creationOther() public pure returns (bytes memory) {
        return type(Other).creationCode;
    }
}
contract Other {
    function f(uint) public returns (uint);
}
// ----
// TypeError: (131-193): Contract "Other" should be marked as abstract.
// TypeError: (97-121): Member "creationCode" not found or not visible after argument-dependent lookup in type(contract Other).
