contract C {
    function f() pure external {
        function() external two_stack_slots;
        assembly {
            let x :=  two_stack_slots
        }
    }
}
// ----
// TypeError 9857: (132-147): Only types that use one stack slot are supported.
