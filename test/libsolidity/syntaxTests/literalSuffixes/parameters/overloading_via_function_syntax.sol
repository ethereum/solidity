function suffix(uint) pure suffix returns (int) {}
function suffix(bool) pure suffix returns (bool) {}
function suffix(address) pure suffix returns (address) {}
function suffix(string memory) pure suffix returns (string memory) {}

contract C {
    int a = suffix(1);
    bool b = suffix(true);
    address c = suffix(0x1234567890123456789012345678901234567890);
    string d = suffix("a");
}
