==== Source: a ====
contract A {}
==== Source: dir/a/b/c ====
import "../../.././a" as x; contract B is x.A { fallback() external { x.A r = x.A(20); r; } }
