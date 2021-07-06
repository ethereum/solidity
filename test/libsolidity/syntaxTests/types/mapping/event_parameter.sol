contract C {
    event E (mapping (uint => uint) [2]);
}
// ----
// TypeError 3448: (26-52): Type containing a (nested) mapping is not allowed as event parameter type.
