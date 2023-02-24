function calldataSuffix(string memory s) pure suffix returns (string memory) {}
function memorySuffix(string calldata s) pure suffix returns (string memory) {}
function storageSuffix(string storage s) pure suffix returns (string memory) {}

contract C {
    function f() public pure {
        "a" calldataSuffix;
        "a" memorySuffix; // OK
        "a" storageSuffix;
    }
}
// ----
// TypeError 2998: (102-119): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
// TypeError 2998: (183-199): This literal suffix function is not usable as a suffix because no literal is implicitly convertible to its parameter type.
// TypeError 8838: (321-324): The literal cannot be converted to type string calldata accepted by the suffix function.
// TypeError 8838: (353-356): The literal cannot be converted to type string storage pointer accepted by the suffix function.
