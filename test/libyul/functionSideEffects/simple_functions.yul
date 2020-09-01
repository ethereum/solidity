{
    function a() {}
    function f() { mstore(0, 1) }
    function g() { sstore(0, 1) }
    function h() { let x := msize() }
    function i() { let z := mload(0) }
}
// ----
// : movable, movable apart from effects, can be removed, can be removed if no msize
// a: movable, movable apart from effects, can be removed, can be removed if no msize
// f: writes memory
// g: writes storage
// h: can be removed, can be removed if no msize, reads memory
// i: movable apart from effects, can be removed if no msize, reads memory
