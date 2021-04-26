==== Source: A ====

error E();

==== Source: B ====

error E();

==== Source: C ====

import "A" as A;
import "B" as B;

contract Test {
    function f() public pure {
        revert A.E();
    }
    function g() public pure {
        revert B.E();
    }
}
// ----
