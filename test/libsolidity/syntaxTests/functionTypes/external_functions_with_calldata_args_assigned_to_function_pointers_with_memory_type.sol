contract C {
    function g(string calldata) external {}

    function main() view external {
        function (string memory) external ptr = this.g;
        ptr;
    }
}
// ----
