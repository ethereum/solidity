{
    function a() {
        if calldataload(0) {
            revert(0, 0)
        }
        reg()
        b()
    }
    function b() {
        a()
        return(0, 0)
    }
    function c() {
        c()
        revert(0, 0)
    }
    function d() {
        switch calldataload(0)
        case 0 { x() }
        case 1 { y() reg() revert(0, 0) }
        default { z() }
    }
    function x() { d() revert(0, 0) }
    function y() { reg() x() }
    function z() { y() }

    function reg() {}
}
// ----
// a: can revert
// b: can revert
// c:
// d:
// x:
// y:
// z:
// reg: can continue
