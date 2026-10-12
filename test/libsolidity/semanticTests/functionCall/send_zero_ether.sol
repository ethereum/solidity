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
// ----
// constructor(), 20 wei ->
// gas irOptimized: 56298
// gas irOptimized code: 37200
// gas legacy: 57555
// gas legacy code: 53000
// gas legacyOptimized: 56463
// gas legacyOptimized code: 39600
// s() -> true
