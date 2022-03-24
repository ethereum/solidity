interface testInterface {
    function A(address payable) external;
}

contract testContract {
    function main() external view {
        abi.encodeCall(testInterface.A, (address(0)));
    }
}
// ----
// TypeError 5407: (171-183): Cannot implicitly convert component at position 0 from "address" to "address payable".
