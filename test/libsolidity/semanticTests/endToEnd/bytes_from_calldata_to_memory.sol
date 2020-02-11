contract C {
    function f() public returns(bytes32) {
        return keccak256(abi.encodePacked("abc", msg.data));
    }
}

// ----
