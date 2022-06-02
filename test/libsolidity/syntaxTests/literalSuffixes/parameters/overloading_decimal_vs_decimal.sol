function uSuffix(uint8, uint) pure returns (int) {}
function uSuffix(uint16, uint) pure returns (int) {}

function iSuffix(int8, uint) pure returns (int) {}
function iSuffix(int16, uint) pure returns (int) {}

function iuSuffix(uint8, uint) pure returns (int) {}
function iuSuffix(int8, uint) pure returns (int) {}

contract C {
    int a = 1.024 uSuffix;  // TODO: Should match only (uint16, uint)
    int b = 1.024 iSuffix;  // TODO: Should match only (int16, uint)

    int c = -1.024 uSuffix; // TODO: Should match only (uint16, uint)
    int d = -1.024 iSuffix; // TODO: Should match only (int16, uint)

    int e = 2.55 iuSuffix;  // TODO: Should match only (uint8, uint)
    int f = -2.55 iuSuffix; // TODO: Should match only (uint8, uint)
}
// ----
// DeclarationError 7920: (347-354): Identifier not found or not unique.
