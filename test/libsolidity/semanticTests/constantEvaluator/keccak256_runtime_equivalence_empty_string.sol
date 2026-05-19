uint constant EMPTY = uint(keccak256(""));

contract C {
    uint[EMPTY] array;

    function testEmptyStringRuntimeEquivalence() public view returns (bool) {
        uint runtime = uint(keccak256(""));

        return array.length == runtime;
    }
}
// ----
// testEmptyStringRuntimeEquivalence() -> true

