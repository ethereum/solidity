function calldataSuffix(string memory s) pure returns (string memory) {}
function memorySuffix(string calldata s) pure returns (string memory) {}
function storageSuffix(string storage s) pure returns (string memory) {}

contract C {
    function f() public pure {
        "a" calldataSuffix;
        "a" memorySuffix;
        "a" storageSuffix;
    }
}
// ----
// TypeError 8838: (300-316): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (326-343): The type of the literal cannot be converted to the parameter of the suffix function.
