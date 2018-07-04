contract C {
    function f() view public {
        bytes32 x = keccak256("abc");
        bytes32 y = sha256("abc");
        address z = ecrecover(bytes32(1), uint8(2), bytes32(3), bytes32(4));
        require(true);
        assert(true);
        x; y; z;
    }
    function g() public {
        bytes32 x = keccak256("abc");
        bytes32 y = sha256("abc");
        address z = ecrecover(bytes32(1), uint8(2), bytes32(3), bytes32(4));
        require(true);
        assert(true);
        x; y; z;
    }
}
// ----
// Warning: (17-261): Function state mutability can be restricted to pure
// Warning: (266-505): Function state mutability can be restricted to pure
