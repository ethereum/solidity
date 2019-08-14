{
    if calldataload(0)
    {
        f()
    }
    g()

    function f() {
        pop(mload(0))
    }
    function g() {
        if sload(0)
        {
            h()
        }
    }
    function h() {
        switch t()
        case 1 {
            i()
        }
    }
    function t() -> x {
        mstore(0, 1)
    }
    function i() {
        sstore(0, 1)
    }
    function r(a) -> b {
        b := mul(a, 2)
    }
}
// ----
// : invalidatesStorage, invalidatesMemory
// f: sideEffectFreeIfNoMSize
// g: invalidatesStorage, invalidatesMemory
// h: invalidatesStorage, invalidatesMemory
// i: invalidatesStorage
// r: movable, sideEffectFree, sideEffectFreeIfNoMSize
// t: invalidatesMemory
