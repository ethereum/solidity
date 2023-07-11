type U8 is uint8;

function checkedAdd(U8 x, U8 y) pure returns (U8) {
    return U8.wrap(U8.unwrap(x) + U8.unwrap(y));
}

using {checkedAdd as +} for U8 global;

contract C {
    function testCheckedOperator() public pure returns (U8) {
        return U8.wrap(250) + U8.wrap(10);
    }

    function testCheckedOperatorInUncheckedBlock() public pure returns (U8) {
        unchecked {
            return U8.wrap(250) + U8.wrap(10);
        }
    }
}
// ----
// testCheckedOperator() -> FAILURE, hex"4e487b71", 0x11
// testCheckedOperatorInUncheckedBlock() -> FAILURE, hex"4e487b71", 0x11
