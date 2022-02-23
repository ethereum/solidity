==== Source: a ====
import "b";
uint constant c = d;
==== Source: b ====
import "a";
uint constant b = c;
uint constant d = b;
contract C {
    uint constant a = b;
}
// ----
// TypeError 6161: (b:12-31): The value of the constant b has a cyclic dependency via c.
// TypeError 6161: (b:33-52): The value of the constant d has a cyclic dependency via b.
// TypeError 6161: (b:71-90): The value of the constant a has a cyclic dependency via b.
// TypeError 6161: (a:12-31): The value of the constant c has a cyclic dependency via d.
