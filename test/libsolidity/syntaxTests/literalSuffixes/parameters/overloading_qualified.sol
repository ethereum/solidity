==== Source: A.sol ====
function suffix(int, uint) pure suffix returns (int) {}
function suffix(bool) pure suffix returns (bool) {}
function suffix(address) pure suffix returns (address) {}
function suffix(string memory) pure suffix returns (string memory) {}

==== Source: B.sol ====
import "A.sol" as A;

contract C {
    int a = 1 A.suffix;
    bool b = true A.suffix;
    address c = 0x1234567890123456789012345678901234567890 A.suffix;
    string d = "a" A.suffix;
}
