==== Source: A ====
function inc(uint x) pure returns (uint) {
    return x + 1;
}

==== Source: B ====
contract C {
	function f(uint x) public returns (uint) {
        return x.f() + x.inc();
    }
}
using {A.inc, f} for uint;
import {inc as f} from "A";
import "A" as A;
// ====
// compileViaYul: also
// ----
// f(uint256): 5 -> 12
// f(uint256): 10 -> 0x16
