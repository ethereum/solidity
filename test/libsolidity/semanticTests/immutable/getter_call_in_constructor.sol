contract A {
    uint immutable public x = 1;
    uint public y;
    constructor() {
        y = this.x();
    }
}
contract C {
    function f() public returns (bool) {
        try new A() { return false; }
        catch { return true; }
    }
}
// ====
// compileViaYul: also
// EVMVersion: >=tangerineWhistle
// ----
// f() -> true
