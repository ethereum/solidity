contract helper {
    function getBalance() payable public returns(uint256 myBalance) {
        return address(this).balance;
    }
}
contract test {
    helper h;
    constructor() public payable {
        h = new helper();
    }

    function sendAmount(uint amount) public returns(uint256 bal) {
        return h.getBalance.value(amount).gas(1000).value(amount + 3)(); // overwrite value
    }
}

// ----
sendAmount(uint256): "5"
