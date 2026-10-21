contract C {
    uint256 constant X = 0x1234;

    function getInnerX() internal returns (uint) { return X; }
    function f() public returns (uint, uint) { return (getInnerX(), getOuterX()); }
}

uint256 constant X = 0x5678;

function getOuterX() returns (uint) { return X; }
// ----
// f() -> 0x1234, 0x5678
