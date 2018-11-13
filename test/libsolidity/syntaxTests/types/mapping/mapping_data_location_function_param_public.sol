contract c {
    function f3(mapping(uint => uint) memory) view public {}
}
// ----
// TypeError: (29-57): Type is required to live outside storage.
// TypeError: (29-57): Internal or recursive type is not allowed for public or external functions.
