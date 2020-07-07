// Sending zero ether to a contract should still invoke the receive ether function
// (it previously did not because the gas stipend was not provided by the EVM)
contract Receiver {
    receive() external payable {}
}


contract Main {
    constructor() payable {}

    function s() public returns (bool) {
        Receiver r = new Receiver();
        return address(r).send(0);
    }
}

// ====
// compileViaYul: also
// ----
// constructor(), 20 wei ->
// s() -> true
