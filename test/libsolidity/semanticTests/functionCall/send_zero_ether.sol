// Sending zero ether to a contract should still invoke the receive ether function
// (it previously did not because the gas stipend was not provided by the EVM)
contract Receiver {
    receive() external payable {}
}


contract Main {
    constructor() payable {}

    function s() public returns (bool) {
        Receiver r = new Receiver();
        return payable(r).send(0);
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// constructor(), 20 wei ->
// gas irOptimized: 101782
// gas legacy: 116691
// gas legacyOptimized: 100361
// s() -> true
