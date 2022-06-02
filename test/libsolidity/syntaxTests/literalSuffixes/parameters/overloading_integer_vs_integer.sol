function uSuffix(uint8) pure returns (int) {}
function uSuffix(uint16) pure returns (int) {}

function iSuffix(int8) pure returns (int) {}
function iSuffix(int16) pure returns (int) {}

function iuSuffix(uint8) pure returns (int) {}
function iuSuffix(int8) pure returns (int) {}

contract C {
    int a = 1024 uSuffix;  // TODO: Should match only uint16
    int b = 1024 iSuffix;  // TODO: Should match only int16

    int c = -1024 uSuffix; // TODO: Should match only uint16
    int d = -1024 iSuffix; // TODO: Should match only int16

    int e = 255 iuSuffix;  // TODO: Should match only uint8
    int f = -255 iuSuffix; // TODO: Should match only uint8
}
// ----
// DeclarationError 7920: (310-317): Identifier not found or not unique.
