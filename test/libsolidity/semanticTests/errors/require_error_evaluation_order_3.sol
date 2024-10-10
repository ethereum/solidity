contract C {
    string failureMessage = "Failure Message";
    function g(bool x) internal returns (bool) {
        failureMessage = "Intercepted failure message";
        return x;
    }
    function h() internal returns (string memory) { return failureMessage; }
    function f(bool c) public {
        require(g(c), h());
    }
}

// ----
// f(bool): false -> FAILURE, hex"08c379a0", 0x20, 0x1b, "Intercepted failure message"
// f(bool): true ->
