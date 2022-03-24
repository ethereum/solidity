interface testInterface {
    function B(function (string calldata) external) external;
}

contract testContract {
    function g(string calldata) external {}
    function h(string memory) external {}

    function main() external view {
        abi.encodeCall(testInterface.B, (this.g));
        abi.encodeCall(testInterface.B, (this.h));
    }
}
// ----
// TypeError 5407: (278-286): Cannot implicitly convert component at position 0 from "function (string memory) external" to "function (string calldata) external".
// TypeError 5407: (329-337): Cannot implicitly convert component at position 0 from "function (string memory) external" to "function (string calldata) external".
