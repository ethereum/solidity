struct S { uint a; string b; }
error E(uint a, S, uint b);
contract C {
    S s;
    function f(bool c) public {
        s.a = 9;
        s.b = "abc";
        require(c, E(2, s, 7));
    }
}
// ====
// compileViaYul: also
// ----
// f(bool): true ->
// f(bool): false -> FAILURE, hex"e96e07f0", 2, 0x60, 7, 9,0x40, 3, "abc"
