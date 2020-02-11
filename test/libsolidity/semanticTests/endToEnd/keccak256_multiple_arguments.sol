contract c {
    function foo(uint a, uint b, uint c) public returns(bytes32 d) {
        d = keccak256(abi.encodePacked(a, b, c));
    }
}

// ----
// foo(uint256,uint256,uint256): 10, 12, 13 ->  util::keccak256( toBigEndian(10 + toBigEndian(12 + toBigEndian(13  
// foo(uint256,uint256,uint256):"10, 12, 13" -> "bc740a98aae5923e8f04c9aa798c9ee82f69e319997699f2782c40828db9fd81"
