type UInt is uint8;
using {add as +} for UInt;
function add(UInt, UInt) pure returns (UInt) {
    return UInt.wrap(0);
}
contract C {
    function f() public pure {
        UInt.wrap(0) + UInt.wrap(1);
    }
}

// ----
