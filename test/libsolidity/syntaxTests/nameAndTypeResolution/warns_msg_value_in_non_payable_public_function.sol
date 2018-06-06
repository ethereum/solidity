contract C {
    function f() view public {
        msg.value;
    }
}
// ----
// Warning: (52-61): "msg.value" used in non-payable function. Do you want to add the "payable" modifier to this function?
