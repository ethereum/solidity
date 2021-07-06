contract c {
    event e(uint indexed a, mapping(uint => uint) indexed b, bool indexed c, uint indexed d, uint indexed e) anonymous;
}
// ----
// TypeError 3448: (41-72): Type containing a (nested) mapping is not allowed as event parameter type.
