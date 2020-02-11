contract c {
    function foo(uint a, uint16 b) public returns(bytes32 d) {
        d = keccak256(abi.encodePacked(a, b, uint8(145)));
    }
}

// ----
// foo(uint256,uint16): 10, 12 ->  util::keccak256( toBigEndian(10 + bytes{0x0, 0xc} + bytes(1, 0x91  
// foo(uint256,uint16):"10, 12" -> "88acd45f75907e7c560318bc1a5249850a0999c4896717b1167d05d116e6dbad"
