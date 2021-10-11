==== Source: A ====
function id(uint x) pure returns (uint) {
    return x;
}

==== Source: B ====
import "A" as M;

contract C {
    using {M.id} for uint;
    function f(uint x) public pure returns (uint) {
        return x.id();
    }
}
