contract C {
    function f() public returns(string memory, uint) {
        return ("abc", 8);
    }

    function g() public returns(string memory, string memory) {
        return (h(), "def");
    }

    function h() public returns(string memory) {
        return ("abc");
    }
}

// ----
// f() -> 0x40, 8, 3, "abc"
// g() -> 0x40, 0x80, 3, "abc", 3, "def"
