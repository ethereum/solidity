contract c {
    function foo() public returns(bytes32 d) {
        d = keccak256("foo");
    }

    function bar(uint a, uint16 b) public returns(bytes32 d) {
        d = keccak256(abi.encodePacked(a, b, uint8(145), "foo"));
    }
}

// ----
// foo() -> util::keccak256("foo"
// foo():"" -> "41b1a0649752af1b28b3dc29a1556eee781e4a4c3a1f7f53f90fa834de098c4d"
// bar(uint256,uint16): 10, 12 ->  util::keccak256( toBigEndian(10 + bytes{0x0, 0xc} + bytes(1, 0x91 + bytes{0x66, 0x6f, 0x6f}  
// bar(uint256,uint16):"10, 12" -> "6990f36476dc412b1c4baa48e2d9f4aa4bb313f61fda367c8fdbbb2232dc6146"
