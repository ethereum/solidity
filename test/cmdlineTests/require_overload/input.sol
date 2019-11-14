pragma solidity >=0.0;
contract C {
    function f() public pure {
        require(this);
    }
}
