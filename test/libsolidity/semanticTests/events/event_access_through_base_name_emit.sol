contract A {
    event x();
}
contract B is A {
    function f() public returns (uint) {
        emit A.x();
        return 1;
    }
}
// ----
// f() -> 1
// ~ emit x()
