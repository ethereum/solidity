contract C {
    function f(string memory a, string memory b, string memory c) public returns (string memory) {
        return string.concat(string.concat(a, b), c);
    }
}
// ====
// compileViaYul: also
// ----
// f(string,string,string): 0x60, 0x60, 0x60, 2, "ab" -> 0x20, 6, "ababab"
