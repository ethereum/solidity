contract C {
    function f(uint256[] memory a, uint256[1] calldata b) public returns (bytes memory) {
        return abi.encode(a, b);
    }

    function g(uint256[] memory a, uint256[1] calldata b) external returns (bytes memory) {
        return f(a, b);
    }

    function h(uint256[] memory a, uint256[1] calldata b) external returns (uint256[] memory, uint256[1] calldata) {
        return (a, b);
    }
}

// ----
// f(uint256[],uint256[1]): 0x40, 0xff, 1, 0xffff -> 0x20, 0x80, 0x40, 0xff, 1, 0xffff
// g(uint256[],uint256[1]): 0x40, 0xff, 1, 0xffff -> 0x20, 0x80, 0x40, 0xff, 1, 0xffff
// h(uint256[],uint256[1]): 0x40, 0xff, 1, 0xffff -> 0x40, 0xff, 1, 0xffff
