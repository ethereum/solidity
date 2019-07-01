contract test {
    function a() public returns(uint n) { return 0; }
    function b() public returns(uint n) { return 1; }
    function c() public returns(uint n) { return 2; }
    function f() public returns(uint n) { return 3; }
}
// ----
// a() -> 0
// b() -> 1
// c() -> 2
// f() -> 3
// i_am_not_there() -> FAILURE
