==== Source: a ====
contract A {} contract B {} contract C { struct S { uint a; } }
==== Source: b ====
import "a" as x; import {B as b, C as c, C} from "a";
contract D is b { function f(c.S memory var1, x.C.S memory var2, C.S memory var3) internal {} }