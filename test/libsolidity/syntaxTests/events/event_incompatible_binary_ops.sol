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
// TypeError 2271: (83-113): Binary operator << not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (123-153): Binary operator >> not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (163-192): Binary operator ^ not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (202-231): Binary operator | not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (241-270): Binary operator & not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (281-310): Binary operator * not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (320-349): Binary operator / not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (359-388): Binary operator % not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (398-427): Binary operator + not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (437-466): Binary operator - not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (477-507): Binary operator == not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (517-547): Binary operator != not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (557-587): Binary operator >= not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (597-627): Binary operator <= not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (637-666): Binary operator < not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (676-705): Binary operator > not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (716-746): Binary operator || not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (756-786): Binary operator && not compatible with types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
