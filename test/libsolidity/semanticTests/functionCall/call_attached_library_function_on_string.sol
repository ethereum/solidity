library D { function length(string memory self) public returns (uint) { return bytes(self).length; } }
contract C {
    using D for string;
    string x;
    function f() public returns (uint) {
        x = "abc";
        return x.length();
    }
    function g() public returns (uint) {
        string memory s = "abc";
        return s.length();
    }
}
// ----
// library: D
// f() -> 3
// g() -> 3
