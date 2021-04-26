contract C {
    string public tester;

    function f() public returns (string memory) {
        return (["abc", "def", "g"][0]);
    }

    function test() public {
        tester = f();
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// test() ->
// tester() -> 0x20, 0x3, "abc"
