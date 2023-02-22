type U8 is uint8;
using {f as ~, add as +} for U8 global;

function f(U8 x) pure returns (U8 z) {
    assembly {
        // NOTE: Not using shr so that the test works pre-constantinople too
        z := div(x, 256)
    }
}

function add(U8 x, U8 y) pure returns (U8 z) {
    assembly {
        z := add(div(x, 256), div(x, 256))
    }
}

contract C {
    function testUnary() external pure returns (U8, U8) {
        U8 a;
        assembly {
            a := 0x4200
        }
        // If the result is not 0, no cleanup was performed.
        return (~a, f(a));
    }

    function testBinary() external pure returns (U8, U8) {
        U8 a;
        U8 b;
        assembly {
            a := 0x4200
            b := 0x4200
        }
        // If the result is not 0, no cleanup was performed.
        return (a + b, add(a, b));
    }
}
// ----
// testUnary() -> 0x42, 0x42
// testBinary() -> 0x84, 0x84
