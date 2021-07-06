contract A {
    event x();
}
contract B is A {
    function f() public returns (uint) {
        emit A.x();
        return 1;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 1
// ~ emit x()
