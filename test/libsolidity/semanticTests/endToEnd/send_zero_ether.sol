contract Receiver {
    receive() external payable {}
}
contract Main {
    constructor() public payable {}

    function s() public returns(bool) {
        Receiver r = new Receiver();
        return address(r).send(0);
    }
}

// ----
// s() -> true
