contract C {
    function f(int256 input) public returns (bytes32 sha256hash) {
        return keccak256(abi.encodePacked(bytes32(uint256(input))));
    }
}
// ----
// f(int256): 4 -> 0x8a35acfbc15ff81a39ae7d344fd709f28e8600b4aa8c65c6b64bfe7fe36bd19b
// f(int256): 5 -> 0x036b6384b5eca791c62761152d0c79bb0604c104a5fb6f4eb0703f3154bb3db0
// f(int256): -1 -> 0xa9c584056064687e149968cbab758a3376d22aedc6a55823d1b3ecbee81b8fb9
