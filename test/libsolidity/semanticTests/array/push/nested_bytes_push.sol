pragma abicoder               v2;
contract C {
    bytes[] a;

    function f() public {
        a.push("abc");
        a.push("def");
        assert(a[0][0] == "a");
        assert(a[1][0] == "d");
    }
}
// ====
// compileViaYul: also
// ----
// f() ->