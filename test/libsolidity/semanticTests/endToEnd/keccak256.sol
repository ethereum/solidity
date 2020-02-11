contract test {
    function a(bytes32 input) public returns(bytes32 hash) {
        return keccak256(abi.encodePacked(input));
    }
}

// ----
// a(bytes32): 0x4 -> 0x8a35acfbc15ff81a39ae7d344fd709f28e8600b4aa8c65c6b64bfe7fe36bd19b
// a(bytes32): 0x5 -> 0x36b6384b5eca791c62761152d0c79bb0604c104a5fb6f4eb0703f3154bb3db0
