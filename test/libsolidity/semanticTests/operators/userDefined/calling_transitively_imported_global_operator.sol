==== Source: s1.sol ====
type Int is int;

using {add1 as +} for Int global;

function add1(Int, Int) pure returns (Int) {
    return Int.wrap(3);
}

==== Source: s2.sol ====
import {Int} from "s1.sol";

using {add2 as +} for Int;

function add2(Int, Int) pure returns (Int) {
    return Int.wrap(7);
}

==== Source: s3.sol ====
import {Int} from "s2.sol";

contract C {
    function f() pure public returns (Int) {
        return Int.wrap(0) + Int.wrap(0);
    }
}
// ----
// f() -> 3
