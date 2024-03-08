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

    function sendAmount(uint256 amount) public payable returns (uint256 bal) {
        uint256 someStackElement = 20;
        return h.getBalance{value: amount + 3, gas: 1000}();
    }
}
// ----
// constructor(), 20 wei ->
// gas irOptimized: 114463
// gas irOptimized code: 58800
// gas legacy: 120076
// gas legacy code: 132200
// gas legacyOptimized: 114536
// gas legacyOptimized code: 65800
// sendAmount(uint256): 5 -> 8
