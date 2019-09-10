contract C {
    uint256 public stateDecimal = 0x20;
}

contract D {
    bool public stateBool = true;
    uint256 public stateDecimal = 42;
    bytes32 public stateBytes = "\x42\x00\xef";

    function internalStateDecimal() public returns (uint256) {
        return (new C()).stateDecimal();
    }

    function update(bool _bool, uint256 _decimal, bytes32 _bytes) public returns (bool, uint256, bytes32) {
        stateBool = _bool;
        stateDecimal = _decimal;
        stateBytes = _bytes;
        return (stateBool, stateDecimal, stateBytes);
    }
}
// ----
// stateBool() -> true
// stateBool() -> right(true)
// stateDecimal() -> 42
// stateDecimal() -> right(42)
// stateBytes() -> left(0x4200ef)
// internalStateDecimal() -> 0x20
// update(bool,uint256,bytes32): false, -23, left(0x2300ef) -> false, -23, left(0x2300ef)
