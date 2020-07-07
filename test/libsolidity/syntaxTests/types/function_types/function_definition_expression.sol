interface Banana {
    function transfer(address,uint256) external returns(bool);
}

contract Apple {
    function f() public pure {
        Banana.transfer;
    }
}
// ----
// Warning 6133: (141-156): Statement has no effect.
