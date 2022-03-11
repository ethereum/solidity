==== Source: A ====
function sub(uint256 x, uint256 y) pure returns (uint) { return 1; }
function sub(uint256 x) pure returns (uint) { return 2; }
==== Source: B ====
import {sub} from "A";
contract C
{
    function f() public pure returns (uint, uint) {
        return (sub(1, 2), sub(2));
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 1, 2
