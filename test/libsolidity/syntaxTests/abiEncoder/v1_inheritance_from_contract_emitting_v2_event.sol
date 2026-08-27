==== Source: A ====
pragma abicoder               v2;

struct Item {
    uint x;
}

library L {
    event Ev(Item);
}

contract C {
    function foo() public {
        emit L.Ev(Item(1));
    }
}
==== Source: B ====
pragma abicoder v1;
import "A";

contract D is C {}
// ====
// compileViaYul: false
// ----
// Warning 9511: (B:0-19): ABI coder v1 is deprecated and scheduled for removal. Use ABI coder v2 instead.
