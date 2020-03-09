contract helper {
    function getBalance() public payable returns (uint256 myBalance) {
        return address(this).balance;
    }
}


contract test {
    helper h;

    constructor() public payable {
        h = new helper();
    }

    function sendAmount(uint256 amount) public returns (uint256 bal) {
        return h.getBalance.value(amount).gas(1000).value(amount + 3)(); // overwrite value
    }
}

// ----
// constructor(), 20 wei ->
// sendAmount(uint256): 5 -> 8
