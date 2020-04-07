contract A {
    uint immutable public x = 1;
    uint public y;
    constructor() public {
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
// EVMVersion: >=tangerineWhistle
// ----
// f() -> true
