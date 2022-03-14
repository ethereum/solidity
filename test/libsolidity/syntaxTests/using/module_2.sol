==== Source: A ====
function id(uint x) pure returns (uint) {
    return x;
}

==== Source: B ====
import {id as Id} from "A";

contract C {
    using { Id } for uint;
	function f(uint x) public pure returns (uint) {
        return x.Id();
    }
}
