==== Source: L ====

function free1() {}
function free2() {}
library L {
    function g() internal {}
    function h() internal {}
}

==== Source: A ====

import "L";
contract A {
    function f() public {
        (L.g)();
        (free2)();
        (L.h)();
    }
}
contract B {
    function f() public {
        (L.h)();
        (free2)();
    }
}

==== Source: C ====

import "L";
contract C {
    function f() public {
        (L.g)();
        (free2)();
        (L.h)();
    }
}

// ----
