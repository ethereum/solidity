==== Source: A ====
library L {
    function id(uint x) internal pure returns (uint) {
        return x;
    }
    function one_ext(uint) pure external returns(uint) {
        return 1;
    }
    function empty() pure internal {
    }

}

==== Source: B ====
contract C {
    using M.L for uint;
    function f(uint x) public pure returns (uint) {
        return x.id();
    }
    function g(uint x) public pure returns (uint) {
        return x.one_ext();
    }
}

import "A" as M;

// ====
// compileViaYul: also
// ----
// library: "A":L
// f(uint256): 5 -> 5
// f(uint256): 10 -> 10
// g(uint256): 5 -> 1
// g(uint256): 10 -> 1
