contract A {
    uint public x = 2;
    constructor(uint) {}
    function f() public returns(uint) { x = 4; }
}
contract B is A {
    constructor() A(f()) {}
}
// ====
// compileViaYul: false
// ----
// x() -> 4
