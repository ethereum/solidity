{
    function a() { mstore8(0, 32) }
    function f() { a() }
    function g() { sstore(0, 1) } // does not affect memory
    function h() { pop(mload(0)) }
    function i() { pop(msize()) }
}
// ----
// : movable, movable apart from effects, can be removed, can be removed if no msize
// a: writes memory
// f: writes memory
// g: writes storage
// h: movable apart from effects, can be removed if no msize, reads memory
// i: can be removed, can be removed if no msize, reads memory
