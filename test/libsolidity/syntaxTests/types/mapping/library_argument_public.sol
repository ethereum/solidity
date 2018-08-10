library L {
    function f(mapping(uint => uint) storage) public pure {
    }
}
// ----
// TypeError: (27-48): Type is required to live outside storage.
