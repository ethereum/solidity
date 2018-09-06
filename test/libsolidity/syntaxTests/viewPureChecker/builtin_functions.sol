contract C {
    function f() public {
        address(this).transfer(1);
        require(address(this).send(2));
        selfdestruct(address(this));
        (bool success,) = address(this).delegatecall("");
        require(success);
		(success,) = address(this).call("");
        require(success);
    }
    function g() pure public {
        bytes32 x = keccak256("abc");
        bytes32 y = sha256("abc");
        address z = ecrecover(bytes32(uint256(1)), uint8(2), bytes32(uint256(3)), bytes32(uint256(4)));
        require(true);
        assert(true);
        x; y; z;
    }
    function() payable external {}
}
