contract C {
    event MyCustomEvent(uint);
    function f() pure public {
        true ? MyCustomEvent : MyCustomEvent;
    }
}

// ----
// TypeError 9717: (90-103): Invalid mobile type in true expression.
// TypeError 3703: (106-119): Invalid mobile type in false expression.
