contract C {
    function f() public pure {
        bytes.concat(
            hex"00",
            hex"aabbcc",
            unicode"abc",
            "123",
            "abc"
            "123456789012345678901234567890123456789012345678901234567890" // Longer than 32 bytes
        );
    }
}
// ----
