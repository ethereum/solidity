{
    function a() {
        switch calldataload(0)
        case 0 { revert(0, 0) }
    }
    function b() {
        switch calldataload(0)
        case 0 { revert(0, 0) }
        default { revert(0, 0) }
    }
    function c() {
        return(0, 0)
        switch calldataload(0)
        case 0 { revert(0, 0) }
        default { }
    }
    function d() {
        switch calldataload(0)
        case 0 { return(0, 0) }
        default { return(0, 0) }
        revert(0, 0)
    }
    function e() {
        switch calldataload(0)
        case 0 { return(0, 0) }
        revert(0, 0)
    }
    function f() {
        switch calldataload(0)
        case 0 { leave }
        default { leave }
        revert(0, 0)
    }
}
// ----
// a: can revert, can continue
// b: can revert
// c: can terminate
// d: can terminate
// e: can terminate, can revert
// f: can continue
