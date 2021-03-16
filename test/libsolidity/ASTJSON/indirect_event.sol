// SPDX-License-Identifier: GPL-3.0
library L {
    event E();
    function f() internal { emit E(); }
}
contract C {
    event H();
    function g() public { L.f(); emit H(); }
}

// ----
