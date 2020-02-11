contract receiver {
    uint public received;

    function recv(uint x) public {
        received += x + 1;
    }
    fallback() external {
        received = 0x80;
    }
}
contract sender {
    constructor() public {
        rec = new receiver();
    }
    fallback() external {
        savedData = msg.data;
    }

    function forward() public returns(bool) {
        address(rec).call(savedData);
        return true;
    }

    function clear() public returns(bool) {
        delete savedData;
        return true;
    }

    function val() public returns(uint) {
        return rec.received();
    }
    receiver rec;
    bytes savedData;
}

// ----
// recv(uint256): 7 -> bytes(
// recv(uint256):"7" -> ""
// val() -> 0
// val():"" -> "0"
// forward() -> true
// forward():"" -> "1"
// val() -> 8
// val():"" -> "8"
// clear() -> true
// clear():"" -> "1"
// val() -> 8
// val():"" -> "8"
// forward() -> true
// forward():"" -> "1"
// val() -> 0x80
// val():"" -> "128"
