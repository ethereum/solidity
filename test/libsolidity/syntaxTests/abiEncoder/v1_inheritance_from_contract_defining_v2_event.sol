==== Source: A ====
pragma experimental ABIEncoderV2;

struct Item {
    uint x;
}

contract C {
    event Ev(Item);
}
==== Source: B ====
import "A";

contract D is C {}
// ----
