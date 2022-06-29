contract C {
    function f(function (string calldata) external) external {}
    function g(string calldata) external {}

    function main() external {
        function (string calldata) external ptr = this.g;
        abi.encodeCall(this.f, (this.g));
        this.f(this.g);
    }
}
// ----
// TypeError 9574: (161-209): Type function (string memory) external is not implicitly convertible to expected type function (string calldata) external.
// TypeError 5407: (242-250): Cannot implicitly convert component at position 0 from "function (string memory) external" to "function (string calldata) external".
// TypeError 9553: (268-274): Invalid type for argument in function call. Invalid implicit conversion from function (string memory) external to function (string calldata) external requested.
