==== Source: a ====
contract A {}
==== Source: dir/a/b/c ====
import "../../.././a"; contract B is A {}
