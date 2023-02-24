function calldataSuffix(string memory s) pure suffix returns (string memory) {}
function memorySuffix(string calldata s) pure suffix returns (string memory) {}
function storageSuffix(string storage s) pure suffix returns (string memory) {}

contract C {
    function f() public pure {
        "a" calldataSuffix;
        "a" memorySuffix;
        "a" storageSuffix;
    }
}
// ----
// TypeError 8838: (321-337): The type of the literal cannot be converted to the parameter of the suffix function.
// TypeError 8838: (347-364): The type of the literal cannot be converted to the parameter of the suffix function.
