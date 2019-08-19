==== Source: a ====
import "b"; contract C { D d; }
==== Source: b ====
import "a"; contract D { C c; }
