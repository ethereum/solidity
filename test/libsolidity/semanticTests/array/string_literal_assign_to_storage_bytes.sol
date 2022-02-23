contract C {
    bytes public s = "abc";
    bytes public s1 = "abcd";
    function f() public {
        s = "abcd";
        s1 = "abc";
    }
    function g() public {
        (s, s1) = ("abc", "abcd");
    }
}
// ====
// compileViaYul: also
// ----
// s() -> 0x20, 3, "abc"
// s1() -> 0x20, 4, "abcd"
// f() ->
// s() -> 0x20, 4, "abcd"
// s1() -> 0x20, 3, "abc"
// g() ->
// s() -> 0x20, 3, "abc"
// s1() -> 0x20, 4, "abcd"
