// bug #8712
abstract contract B {
    uint immutable x;
    constructor(function() internal returns(uint) fp) {
        x = fp(); }
}
// ----
//     :B
// []
