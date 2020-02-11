contract helper {
    receive() external payable {} // can receive ether
}
contract test {
    helper h;
    constructor() public payable {
        h = new helper();
        address(h).send(5);
    }

    function getBalance() public returns(uint256 myBalance, uint256 helperBalance) {
        myBalance = address(this).balance;
        helperBalance = address(h).balance;
    }
}

// ----
// getBalance() -> 0, 0
