interface testInterface {
    function B(function (string calldata) external) external;
    error E(function (string calldata) external);
}

contract testContract {
    function g(string calldata) external {}
    function h(string memory) external {}

    function main() external view {
        abi.encodeCall(testInterface.B, (this.g));
        abi.encodeCall(testInterface.B, (this.h));
        abi.encodeError(testInterface.E, (this.g));
        abi.encodeError(testInterface.E, (this.h));
    }
}
// ----
// TypeError 5407: (328-336): Cannot implicitly convert component at position 0 from "function (string memory) external" to "function (string calldata) external".
// TypeError 5407: (379-387): Cannot implicitly convert component at position 0 from "function (string memory) external" to "function (string calldata) external".
// TypeError 5407: (431-439): Cannot implicitly convert component at position 0 from "function (string memory) external" to "function (string calldata) external".
// TypeError 5407: (483-491): Cannot implicitly convert component at position 0 from "function (string memory) external" to "function (string calldata) external".
