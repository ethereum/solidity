function suffix(uint x) pure suffix returns (uint) { return x; }

contract C {
    uint x = 10e1000 suffix;
    uint y = 999999999999999999999999999999999999999999999999999999999999999999999999999999 suffix;
}
// ----
// TypeError 8838: (92-99): The number is out of range of type uint256 accepted by the suffix function.
// TypeError 8838: (121-199): The number is out of range of type uint256 accepted by the suffix function.
