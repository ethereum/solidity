contract C {
    function f() public {
        address(this).transfer(1);
        require(address(this).send(2));
        selfdestruct(address(this));
        require(address(this).delegatecall());
        require(address(this).call());
    }
    function g() pure public {
        bytes32 x = keccak256("abc");
        bytes32 y = sha256("abc");
        address z = ecrecover(1, 2, 3, 4);
        require(true);
        assert(true);
        x; y; z;
    }
    function() payable public {}
}
