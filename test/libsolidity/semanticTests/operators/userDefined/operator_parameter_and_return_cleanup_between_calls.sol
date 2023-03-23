type U8 is uint8;
using {yoloAdd as +, yoloDiv as /} for U8 global;

function yoloAdd(U8 x, U8 y) pure returns (U8 z) {
    assembly {
        z := add(x, y) // Wrong! No cleanup.
    }
}

function yoloDiv(U8 x, U8 y) pure returns (U8 z) {
    assembly {
        z := div(x, y) // Wrong! No cleanup.
    }
}

contract C {
    function divAddNoOverflow(U8 a, U8 b, U8 c) external pure returns (U8) {
        return a / (b + c);
    }
}
// ----
// divAddNoOverflow(uint8,uint8,uint8): 4, 0xff, 3 -> 0
