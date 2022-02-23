contract test {
    function f() public pure returns (string memory) {
        return "hello world";
    }
    function g() public pure returns (string memory) {
        return unicode"hello world";
    }
}
// ----
