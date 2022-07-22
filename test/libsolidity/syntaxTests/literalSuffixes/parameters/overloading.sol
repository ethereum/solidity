function suffix(uint) pure returns (int) {}
function suffix(bool) pure returns (bool) {}
function suffix(address) pure returns (address) {}
function suffix(string memory) pure returns (string memory) {}

contract C {
    int a = 1 suffix;
    bool b = true suffix;
    address c = 0x1234567890123456789012345678901234567890 suffix;
    string d = "a" suffix;
}
// ----
