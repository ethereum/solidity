contract test {
    constructor() payable {}

    function getBalance() public returns (uint256 balance) {
        return address(this).balance;
    }
}

// ====
// compileViaYul: also
// ----
// constructor(), 23 wei ->
// getBalance() -> 23
