contract C {
    event MyCustomEvent(uint);
    event MyCustomEvent2(uint);
    event MyCustomEvent3(uint, bool);
    function f() pure public {
        true ? MyCustomEvent : MyCustomEvent;
        true ? MyCustomEvent : MyCustomEvent2;
        true ? MyCustomEvent : MyCustomEvent3;
        true ? MyCustomEvent : true;
        true ? true : MyCustomEvent;
    }
}

// ----
// TypeError 1080: (246-283): True expression's type event MyCustomEvent(uint256) does not match false expression's type event MyCustomEvent3(uint256,bool).
// TypeError 1080: (293-320): True expression's type event MyCustomEvent(uint256) does not match false expression's type bool.
// TypeError 1080: (330-357): True expression's type bool does not match false expression's type event MyCustomEvent(uint256).
