library L {
    function f(mapping(uint => uint) storage) public pure {
    }
}
// ----
// TypeError: (27-56): Type is required to live outside storage.
