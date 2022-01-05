{
    f(calldataload(0), calldataload(32))
    h(calldataload(64), calldataload(96))

    function f(a, b) {
        // gets removed
        sstore(a, b)
        g()
        sstore(a, b)
    }

    function g() {
        pop(staticcall(0, 0, 0, 0, 0, 0))
    }

    function h(a_, b_) {
        // cannot be removed
        sstore(a_, b_)
        i()
        sstore(a_, b_)
    }

    function i() {
        pop(delegatecall(0, 0, 0, 0, 0, 0))
    }


}
// ====
// EVMVersion: >=byzantium
// ----
// step: equalStoreEliminator
//
// {
//     f(calldataload(0), calldataload(32))
//     h(calldataload(64), calldataload(96))
//     function f(a, b)
//     {
//         sstore(a, b)
//         g()
//     }
//     function g()
//     {
//         pop(staticcall(0, 0, 0, 0, 0, 0))
//     }
//     function h(a_, b_)
//     {
//         sstore(a_, b_)
//         i()
//         sstore(a_, b_)
//     }
//     function i()
//     {
//         pop(delegatecall(0, 0, 0, 0, 0, 0))
//     }
// }
