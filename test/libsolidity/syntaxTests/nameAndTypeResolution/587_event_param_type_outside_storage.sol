contract c {
    event e(uint indexed a, mapping(uint => uint) indexed b, bool indexed c, uint indexed d, uint indexed e) anonymous;
}
// ----
// TypeError: (41-72): Type is required to live outside storage.
// TypeError: (41-72): Internal or recursive type is not allowed as event parameter type.
// TypeError: (17-132): More than 4 indexed arguments for anonymous event.
