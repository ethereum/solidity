contract C {
    function transient() public pure { }
}

error CustomError(uint transient);
event e1(uint transient);
event e2(uint indexed transient);

struct S {
    int transient;
}

contract D {
    function f() public pure returns (uint) {
        uint transient = 1;
        return transient;
    }

    function g(int transient) public pure { }

    modifier m(address transient) {
        _;
    }
}
// ----
