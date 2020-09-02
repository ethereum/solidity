contract C {
    uint256 value;
    constructor() {
        value = msg.value;
    }
}
// ----
// TypeError 5887: (68-77): "msg.value" and "callvalue()" can only be used in payable constructors. Make the constructor "payable" to avoid this error.
