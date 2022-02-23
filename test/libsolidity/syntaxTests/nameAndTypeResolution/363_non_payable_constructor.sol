contract C {
    constructor() { }
}
contract D {
    function f() public returns (uint) {
        (new C){value: 2}();
        return 2;
    }
}
// ----
// TypeError 7006: (99-116): Cannot set option "value", since the constructor of contract C is not payable.
