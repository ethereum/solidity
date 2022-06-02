==== Source: A.sol ====
function s(uint) pure returns (uint) { return 1; }
==== Source: B.sol ====
import {s} from "A.sol";
import {s as z} from "A.sol";
import "A.sol" as A;
import "B.sol" as B;

contract C {
    function f() pure public {
        1 s;
        2 z;
        3 A.s;
        4 B.B.B.A.s;
    }
}
