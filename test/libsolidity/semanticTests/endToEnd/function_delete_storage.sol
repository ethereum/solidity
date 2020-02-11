contract C {
    function a() public returns(uint) {
        return 7;
    }

    function() internal returns(uint) y;

    function set() public returns(uint) {
        y = a;
        return y();
    }

    function d() public returns(uint) {
        delete y;
        return 1;
    }

    function ca() public returns(uint) {
        return y();
    }
}

// ----
// set() -> 7
// set():"" -> "7"
// ca() -> 7
// ca():"" -> "7"
// d() -> 1
// d():"" -> "1"
// ca() -> 
// ca():"" -> ""
