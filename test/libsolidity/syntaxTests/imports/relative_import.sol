==== Source: a ====
import "./dir/b"; contract A is B {}
==== Source: dir/b ====
contract B {}
==== Source: dir/c ====
import "../a"; contract C is A {}