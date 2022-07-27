contract C {
    function f(uint256[] memory a, bytes calldata b) public returns (bytes memory) {
        return abi.encode(a, b);
    }

    function g(uint256[] memory a, bytes calldata b) external returns (bytes memory) {
        return f(a, b);
    }
}

// ----
// f(uint256[],bytes): 0x40, 0x80, 1, 0xFF, 6, "123456" -> 0x20, 0xc0, 0x40, 0x80, 1, 0xff, 6, "123456"
// g(uint256[],bytes): 0x40, 0x80, 1, 0xffff, 8, "12345678" -> 0x20, 0xc0, 0x40, 0x80, 1, 0xffff, 8, "12345678"
