type U8 is uint8;
using {f as ~, g as +} for U8 global;

function f(U8) pure returns (U8 z) {
    assembly {
        // Return a value with dirty bytes outside of uint8
        z := 0xffff
    }
}

function g(U8, U8) pure returns (U8 z) {
    assembly {
        // Return a value with dirty bytes outside of uint8
        z := 0xffff
    }
}

contract C {
    function testUnary() external pure returns (uint, uint) {
        U8 a; // Value does not matter

        U8 opResult = ~a;
        U8 fResult = f(a);

        // Get the slot, including bytes outside of uint8
        uint opResultFull;
        uint fResultFull;
        assembly {
            opResultFull := opResult
            fResultFull := fResult
        }

        // If the result is not 0xff, no cleanup was performed.
        return (opResultFull, fResultFull);
    }

    function testBinary() external pure returns (uint, uint) {
        U8 a; // Value does not matter
        U8 b; // Value does not matter

        U8 opResult = a + b;
        U8 fResult = g(a, b);

        // Get the slot, including bytes outside of uint8
        uint opResultFull;
        uint fResultFull;
        assembly {
            opResultFull := opResult
            fResultFull := fResult
        }

        // If the result is not 0xff, no cleanup was performed.
        return (opResultFull, fResultFull);
    }
}
// ----
// testUnary() -> 0xffff, 0xffff
// testBinary() -> 0xffff, 0xffff
