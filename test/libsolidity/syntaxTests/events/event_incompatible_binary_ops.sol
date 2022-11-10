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
// TypeError 2271: (83-113): Built-in binary operator << cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (123-153): Built-in binary operator >> cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (163-192): Built-in binary operator ^ cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (202-231): Built-in binary operator | cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (241-270): Built-in binary operator & cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (281-310): Built-in binary operator * cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (320-349): Built-in binary operator / cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (359-388): Built-in binary operator % cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (398-427): Built-in binary operator + cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (437-466): Built-in binary operator - cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (477-507): Built-in binary operator == cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (517-547): Built-in binary operator != cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (557-587): Built-in binary operator >= cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (597-627): Built-in binary operator <= cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (637-666): Built-in binary operator < cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (676-705): Built-in binary operator > cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (716-746): Built-in binary operator || cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
// TypeError 2271: (756-786): Built-in binary operator && cannot be applied to types event MyCustomEvent(uint256) and event MyCustomEvent(uint256).
