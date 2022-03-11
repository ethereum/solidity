interface I {
    function f(function (string calldata) external view returns (uint)) external;
}

contract C {
    function g(string memory) external {}

    function main() external view {
        abi.encodeCall(I.f, (this.g));
    }
}
// ----
// TypeError 5407: (219-227): Cannot implicitly convert component at position 0 from "function (string memory) external" to "function (string calldata) view external returns (uint256)".
