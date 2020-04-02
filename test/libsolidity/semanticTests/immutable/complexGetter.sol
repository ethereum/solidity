contract C {
    function() external returns (uint, uint) immutable public x = this.f;
    function f() external pure returns (uint, uint) {
        return (1, 2);
    }

    function test() external returns (uint, uint) {
        return this.x()();
    }
}
// ----
// test() -> 1, 2
