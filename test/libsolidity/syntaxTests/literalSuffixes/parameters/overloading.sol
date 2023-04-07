function suffix(uint) pure suffix returns (int) {}
function suffix(bool) pure suffix returns (bool) {}
function suffix(address) pure suffix returns (address) {}
function suffix(string memory) pure suffix returns (string memory) {}

contract C {
    int a = 1 suffix;
}
// ----
// TypeError 2144: (259-265): No matching declaration found after variable lookup.
