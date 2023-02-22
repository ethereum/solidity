==== Source: A ====
type global is uint;
using { f } for global global;
function f(global x) pure returns (global) { return global.wrap(global.unwrap(x) + 1); }
==== Source: B ====
import { global } from "A";

function g(global x) pure returns (global) { return global.wrap(global.unwrap(x) + 10); }

contract C {
    using { g } for global;
    function f(global r) public pure returns (global) {
        return r.f().g();
    }
}
// ----
// f(uint256): 100 -> 111