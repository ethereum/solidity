{
    function a() {
        for { leave } calldataload(0) { } {
            break
            revert(0, 0)
        }
    }
    function b() {
        for { } calldataload(0) { leave } {
            break
            revert(0, 0)
        }
    }
    function b2() {
        for { } calldataload(0) { leave } {
            revert(0, 0)
        }
    }
    function c() {
        for { } calldataload(0) { revert(0, 0) } {
            break
        }
    }
    function c2() {
        for { } calldataload(0) { revert(0, 0) } {
            break
            revert(0, 0)
        }
    }
    function d() {
        for { } calldataload(0) { revert(0, 0) } {
            continue
        }
    }
    function e() {
        for { } calldataload(0) { revert(0, 0) } {
            if calldataload(1) { break }
        }
    }
    function f() {
        for { } calldataload(0) {  } {
            if calldataload(1) { continue }
            revert(0, 0)
        }
    }
}
// ----
// a: can continue
// b: can continue
// b2: can revert, can continue
// c: can continue
// c2: can continue
// d: can revert, can continue
// e: can revert, can continue
// f: can revert, can continue
