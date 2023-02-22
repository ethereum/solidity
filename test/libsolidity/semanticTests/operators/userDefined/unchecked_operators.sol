type U8 is uint8;

function uncheckedAdd(U8 x, U8 y) pure returns (U8) {
    unchecked {
        return U8.wrap(U8.unwrap(x) + U8.unwrap(y));
    }
}

using {uncheckedAdd as +} for U8 global;

contract D {
    function testUncheckedOperator() public pure returns (U8) {
        return U8.wrap(250) + U8.wrap(10);
    }

    function testUncheckedOperatorInUncheckedBlock() public pure returns (U8) {
        unchecked {
            return U8.wrap(250) + U8.wrap(10);
        }
    }
}
// ----
// testUncheckedOperator() -> 4
// testUncheckedOperatorInUncheckedBlock() -> 4
