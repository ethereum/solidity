==== Source: A.sol ====
event EA();
==== Source: B.sol ====
event EB();
==== Source: C.sol ====
import "A.sol";
import {EB} from "B.sol";

function f() {
    emit EA();
    emit EB();
}
