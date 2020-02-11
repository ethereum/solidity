contract C {
    uint x;

    function f() public returns(uint) {
        x = 3;
        return 1;
    }
}
interface CView {
    function f() view external returns(uint);
}
interface CPure {
    function f() pure external returns(uint);
}
contract D {
    function f() public returns(uint) {
        return (new C()).f();
    }

    function fview() public returns(uint) {
        return (CView(address(new C()))).f();
    }

    function fpure() public returns(uint) {
        return (CPure(address(new C()))).f();
    }
}

// ----
// f() -> 1
// f():"" -> "1"
// fview() -> 
// fview():"" -> ""
// fpure() -> 
// fpure():"" -> ""
