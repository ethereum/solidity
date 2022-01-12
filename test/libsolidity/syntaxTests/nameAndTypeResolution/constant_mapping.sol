contract C {
    mapping(uint => uint) constant x;
}
// ----
// TypeError 9259: (17-49): Only constants of value type and byte arrays are implemented.
