contract test {
    uint120[3] x;

    function f() public returns(bytes32 hash1, bytes32 hash2, bytes32 hash3) {
        uint120[] memory y = new uint120[](3);
        x[0] = y[0] = uint120(-2);
        x[1] = y[1] = uint120(-3);
        x[2] = y[2] = uint120(-4);
        hash1 = keccak256(abi.encodePacked(x));
        hash2 = keccak256(abi.encodePacked(y));
        hash3 = keccak256(abi.encodePacked(this.f));
    }
}

// ----
// f() ->  util::keccak256("0xfffffffffffffffffffffffffffffe", "0xfffffffffffffffffffffffffffffd", "0xfffffffffffffffffffffffffffffc", util::keccak256("0xfffffffffffffffffffffffffffffe", "0xfffffffffffffffffffffffffffffd", "0xfffffffffffffffffffffffffffffc", util::keccak256(fromHex(m_contractAddress.hex( + "26121ff0" 
// f():"" -> "ba4f20407251e4607cd66b90bfea19ec6971699c03e4a4f3ea737d5818ac27ae, ba4f20407251e4607cd66b90bfea19ec6971699c03e4a4f3ea737d5818ac27ae, eb9d33b3d30e0cda850322f7dadb11d47d6bf5bf706b73c560ea450a90eba9de"
