contract C {
    function g(bytes calldata b) pure internal {}

    function main() pure external {
        function (bytes calldata) internal ptr = g;
        ptr;
    }
}
// ----
