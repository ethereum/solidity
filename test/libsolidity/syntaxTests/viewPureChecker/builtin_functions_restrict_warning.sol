contract C {
    function f() view public {
        bytes32 x = keccak256("abc");
        bytes32 y = sha256("abc");
        address z = ecrecover(bytes32(uint256(1)), uint8(2), bytes32(uint256(3)), bytes32(uint256(4)));
        require(true);
        assert(true);
        x; y; z;
    }
    function g() public {
        bytes32 x = keccak256("abc");
        bytes32 y = sha256("abc");
        address z = ecrecover(bytes32(uint256(1)), uint8(2), bytes32(uint256(3)), bytes32(uint256(4)));
        require(true);
        assert(true);
        x; y; z;
    }
}
// ----
// Warning: (17-288): Function state mutability can be restricted to pure
// Warning: (293-559): Function state mutability can be restricted to pure
