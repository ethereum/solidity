interface I {
    function f(function (string calldata) external) external;
}

contract C {
    function g(string calldata) external {}

    function main() external view {
        abi.encodeCall(I.f, (this.g));
    }
}
// ----
// TypeError 5407: (201-209): Cannot implicitly convert component at position 0 from "function (string memory) external" to "function (string calldata) external".
