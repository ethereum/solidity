function free1() {}
function free2() {}
function free3() {}
library L {
    function ext() external {}
    function inr1() internal {}
    function inr2() internal {}
    function inr3() internal {}
    function access() public {
        free1;
        inr1;
        L.ext;
    }
    function expression() public {
        (free2)();
        (inr2)();
    }
}
contract C {
    function ext1() external {}
    function ext2() external {}
    function ext3() external {}
    function inr1() internal {}
    function inr2() internal {}
    function inr3() internal {}
    function access() public {
        this.ext1;
        inr1;
        free1;
        L.inr1;
        L.ext;
    }
    function expression() public {
        (this.ext2)();
        (inr2)();
        (free2)();
        (L.inr2)();
        (L.ext)();
    }
}
contract D is C {
    constructor() {
        access();
        expression();
    }
}

// ----
