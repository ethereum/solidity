contract c {
    function f3(mapping(uint => uint) memory) view public {}
}
// ----
// TypeError: (29-50): Type is required to live outside storage.
// TypeError: (29-50): Internal or recursive type is not allowed for public or external functions.
