contract C {
    function g(string calldata) external returns (bool) { return true; }

    function main() external returns (bool) {
        function (string memory) external returns (bool) ptr = this.g;
        return ptr("testString");
    }
}
// ----
// main() -> true
