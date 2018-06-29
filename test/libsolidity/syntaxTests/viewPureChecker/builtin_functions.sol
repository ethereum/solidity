contract C {
    function f() public {
        address(this).transfer(1);
        require(address(this).send(2));
        selfdestruct(address(this));
        require(address(this).delegatecall(""));
        require(address(this).call(""));
    }
    function g() pure public {
        bytes32 x = keccak256("abc");
        bytes32 y = sha256("abc");
        address z = ecrecover(bytes32(1), uint8(2), bytes32(3), bytes32(4));
        require(true);
        assert(true);
        x; y; z;
    }
    function() payable external {}
}
