==== Source: A ====
pragma abicoder               v2;

struct Item {
    uint x;
}

contract C {
    event Ev(Item);
}
==== Source: B ====
pragma abicoder v1;
import "A";

contract D is C {}
// ----
