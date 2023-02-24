function suffix(string memory) pure suffix returns (string memory) {}
function suffix(bytes memory) pure suffix returns (bytes memory) {}

contract C {
    bytes a = hex"abcd" suffix;
}
