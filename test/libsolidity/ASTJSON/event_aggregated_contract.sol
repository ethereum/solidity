contract C {
    event e();
    function f() public {
        emit e();
    }
}
contract D {
    C c;
    constructor() {
        c = new C();
        c.f();
    }
}

// ----
