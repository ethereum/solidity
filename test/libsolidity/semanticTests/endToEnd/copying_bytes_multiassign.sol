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
        savedData1 = savedData2 = msg.data;
    }

    function forward(bool selector) public returns(bool) {
        if (selector) {
            address(rec).call(savedData1);
            delete savedData1;
        } else {
            address(rec).call(savedData2);
            delete savedData2;
        }
        return true;
    }

    function val() public returns(uint) {
        return rec.received();
    }
    receiver rec;
    bytes savedData1;
    bytes savedData2;
}

// ----
// recv(uint256): 7 -> bytes(
// recv(uint256):"7" -> ""
// val() -> 0
// val():"" -> "0"
// forward(bool): true -> true
// forward(bool):"1" -> "1"
// val() -> 8
// val():"" -> "8"
// forward(bool): false -> true
// forward(bool):"0" -> "1"
// val() -> 16
// val():"" -> "16"
// forward(bool): true -> true
// forward(bool):"1" -> "1"
// val() -> 0x80
// val():"" -> "128"
