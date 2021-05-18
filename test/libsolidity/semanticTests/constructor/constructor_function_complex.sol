contract D {
    uint public x;
    constructor(function() external pure returns (uint) g) {
        x = g();
    }
}

contract C {
    function f() public returns (uint r) {
        D d = new D(this.sixteen);
        r = d.x();
    }

    function sixteen() public pure returns (uint) {
        return 16;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 16
// gas legacy: 103744
