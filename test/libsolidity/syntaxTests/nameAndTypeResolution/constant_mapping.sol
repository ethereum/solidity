contract C {
    mapping(uint => uint) constant x;
}
// ----
// DeclarationError 3530: (17-49): The type contains a (nested) mapping and therefore cannot be a constant.
