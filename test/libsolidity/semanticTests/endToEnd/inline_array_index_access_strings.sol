contract C {
    string public tester;

    function f() public returns(string memory) {
        return (["abc", "def", "g"][0]);
    }

    function test() public {
        tester = f();
    }
}

// ----
// test() -> 
// test():"" -> ""
// tester() -> 0x20, 3, string("abc"
// tester():"" -> "32, 3, abc"
