contract C {
    function f() public returns (uint) {
        return uint(this.f);
    }
}
