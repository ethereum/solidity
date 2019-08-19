==== Source: a ====
contract C { }
==== Source: b ====
import "a";
==== Source: c ====
import "b";
contract D is C {}