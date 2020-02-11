contract C {
    function f() public returns(bytes32) {
        return keccak256(abi.encodePacked("abc", msg.data));
    }
}

// ----
// f() -> 0x5bd33cebe907fed0f6b06e5bb063a8953b845729e85ef2dc07bbeabcb45eb391
