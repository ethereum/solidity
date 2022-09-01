contract C {
    event MyCustomEvent(uint);
    function f() pure public {
        MyCustomEvent << MyCustomEvent;
        MyCustomEvent >> MyCustomEvent;
        MyCustomEvent ^ MyCustomEvent;
        MyCustomEvent | MyCustomEvent;
        MyCustomEvent & MyCustomEvent;

        MyCustomEvent * MyCustomEvent;
        MyCustomEvent / MyCustomEvent;
        MyCustomEvent % MyCustomEvent;
        MyCustomEvent + MyCustomEvent;
        MyCustomEvent - MyCustomEvent;

        MyCustomEvent == MyCustomEvent;
        MyCustomEvent != MyCustomEvent;
        MyCustomEvent >= MyCustomEvent;
        MyCustomEvent <= MyCustomEvent;
        MyCustomEvent < MyCustomEvent;
        MyCustomEvent > MyCustomEvent;

        MyCustomEvent || MyCustomEvent;
        MyCustomEvent && MyCustomEvent;
    }
}

// ----
// TypeError 2271: (83-113): Operator << not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (123-153): Operator >> not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (163-192): Operator ^ not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (202-231): Operator | not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (241-270): Operator & not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (281-310): Operator * not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (320-349): Operator / not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (359-388): Operator % not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (398-427): Operator + not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (437-466): Operator - not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (477-507): Operator == not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (517-547): Operator != not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (557-587): Operator >= not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (597-627): Operator <= not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (637-666): Operator < not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (676-705): Operator > not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (716-746): Operator || not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (756-786): Operator && not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
