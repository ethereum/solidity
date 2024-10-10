contract C {
    function () external transient f;
    function g() external {
    }

    function test() public returns (bool) {
        assert(f != this.g);
        f = this.g;

        return f == this.g;
    }
}
// ====
// EVMVersion: >=cancun
// ----
// test() -> true
