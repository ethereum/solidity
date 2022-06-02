function suffix(uint) pure returns (int) {}
function suffix(bool) pure returns (int) {}
function suffix(address) pure returns (int) {}
function suffix(string memory) pure returns (int) {}

contract C {
    int a = 1 suffix;                                          // TODO: Should match only uint
    int b = true suffix;                                       // TODO: Should match only bool
    int c = 0x1234567890123456789012345678901234567890 suffix; // TODO: Should match only address
    int d = "a" suffix;                                        // TODO: Should match only string
}
// ----
// DeclarationError 7920: (216-222): Identifier not found or not unique.
