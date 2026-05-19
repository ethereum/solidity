uint constant UNICODE = uint(keccak256(unicode"Araçá"));

contract C {
    uint[UNICODE] array;

    function testUnicodeStringRuntimeEquivalence() public view returns (bool) {
        uint runtime = uint(keccak256(unicode"Araçá"));

        return array.length == runtime;
    }
}
// ----
// testUnicodeStringRuntimeEquivalence() -> true

