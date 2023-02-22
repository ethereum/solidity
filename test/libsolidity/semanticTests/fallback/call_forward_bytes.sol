contract receiver {
    uint256 public received;
    function recv(uint256 x) public { received += x + 1; }
    fallback() external { received = 0x80; }
}
contract sender {
    constructor() { rec = new receiver();}
    fallback() external { savedData = msg.data; }
    function forward() public returns (bool) { address(rec).call(savedData); return true; }
    function clear() public returns (bool) { delete savedData; return true; }
    function val() public returns (uint) { return rec.received(); }
    receiver rec;
    bytes savedData;
}
// ====
// allowNonExistingFunctions: true
// ----
// recv(uint256): 7 ->
// val() -> 0
// forward() -> true
// val() -> 8
// clear() -> true
// val() -> 8
// forward() -> true
// val() -> 0x80
