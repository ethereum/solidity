==== Source: A.sol ====
function suffix(int, uint) pure suffix returns (int) {}
function suffix(bool) pure suffix returns (bool) {}
function suffix(address) pure suffix returns (address) {}
function suffix(string memory) pure suffix returns (string memory) {}

==== Source: B.sol ====
import "A.sol" as A;

contract C {
    int a = 1 A.suffix;
}
// ----
// TypeError 6675: (B.sol:49-57): Member "suffix" not unique after argument-dependent lookup in module "A.sol".
