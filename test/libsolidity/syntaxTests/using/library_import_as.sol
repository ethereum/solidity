==== Source: A ====
library L {
    function id(uint x) pure internal returns (uint) {
        return x;
    }
}

==== Source: B ====
import {L as M} from "A";

contract C {
    using M for uint;
	function f(uint x) public pure returns (uint) {
        return x.id();
    }
}
