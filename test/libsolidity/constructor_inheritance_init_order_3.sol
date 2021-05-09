contract A {
    uint public x;
    constructor(uint) {}
    function f() public { x = 4; }
}
contract B is A {
    constructor() A(f()) {}
}
// ====
// compileViaYul: also
// ----
// x() -> 4
