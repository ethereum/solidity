contract helper {
    function getBalance() public payable returns (uint256 myBalance) {
        return address(this).balance;
    }
}


contract test {
    helper h;

    constructor() payable {
        h = new helper();
    }

    function sendAmount(uint256 amount) public returns (uint256 bal) {
        return h.getBalance{value: amount + 3, gas: 1000}();
    }
}
// ----
// constructor(), 20 wei ->
// gas irOptimized: 114527
// gas irOptimized code: 59600
// gas legacy: 120199
// gas legacy code: 133600
// gas legacyOptimized: 114568
// gas legacyOptimized code: 66200
// sendAmount(uint256): 5 -> 8
