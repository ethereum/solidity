contract C {
    function f(uint) external {}

    function main() external view {
        function () h;
        abi.encodeCall(this.f, (h));
    }
}
// ----
// TypeError 5407: (137-140): Cannot implicitly convert component at position 0 from "function ()" to "uint256".
