function suffix(string memory) pure returns (string memory) {}
function suffix(bytes memory) pure returns (bytes memory) {}

contract C {
    bytes a = hex"abcd" suffix;
}
