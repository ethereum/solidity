==== Source: M.sol ====
event E();
==== Source: A.sol ====
import "M.sol" as M;

function f() {
    emit M.E();
}
