pragma solidity ^0.4.0;

contract Test {
    address _owner = 0x123456789012345678901234567890123456789012;

    constructor(address owner) public {
        _owner = owner;
    }

    function changeOwner(address owner) public {
        _owner = owner;
    }
}
