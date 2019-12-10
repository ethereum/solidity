==== Source: a ====
contract A
{
	uint constant a = 42;
}
==== Source: b ====
import {A as b} from "a";
contract B {
    function f() public pure {
        assembly {
            let A := 1
            let A.b := 2
        }
    }
}
