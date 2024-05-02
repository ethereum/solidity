interface testInterface {
    function A(address payable) external;
    error E(address payable);
}

contract testContract {
    function main() external view {
        abi.encodeCall(testInterface.A, (address(0)));
        abi.encodeError(testInterface.E, (address(0)));
    }
}
// ----
// TypeError 5407: (201-213): Cannot implicitly convert component at position 0 from "address" to "address payable".
// TypeError 5407: (257-269): Cannot implicitly convert component at position 0 from "address" to "address payable".
