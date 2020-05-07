// bug #8712
contract B {
    uint immutable x;
    constructor(function() internal returns(uint) fp) internal {
        x = fp(); }
}
// ----
//     :B
// []
