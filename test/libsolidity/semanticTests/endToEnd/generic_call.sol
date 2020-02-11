contract receiver {
    uint public received;

    function recv(uint256 x) public payable {
        received = x;
    }
}
contract sender {
    constructor() public payable {}

    function doSend(address rec) public returns(uint d) {
        bytes4 signature = bytes4(bytes32(keccak256("recv(uint256)")));
        rec.call.value(2)(abi.encodeWithSelector(signature, 23));
        return receiver(rec).received();
    }
}

// ----

contract receiver {
    uint public received;

    function recv(uint256 x) public payable {
        received = x;
    }
}
contract sender {
    constructor() public payable {}

    function doSend(address rec) public returns(uint d) {
        bytes4 signature = bytes4(bytes32(keccak256("recv(uint256)")));
        rec.call.value(2)(abi.encodeWithSelector(signature, 23));
        return receiver(rec).received();
    }
}

// ----
doSend(address): "0"
