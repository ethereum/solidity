contract c {
    function f() public pure {
        int a;
        a = (((((4 ** 4) ** 2) ** 4) ** 4) ** 4) ** 4;
        a = -(((4 ** 4 ** 2 ** 4 ** 4) ** 4) ** 4) ** 4;
        a = 4 ** (-(2 ** 4 ** 4 ** 4 ** 4 ** 4));
        a = 2 ** 1E1233;
        a = -2 ** 1E1233;
        a = 2 ** -1E1233;
        a = -2 ** -1E1233;
        a = 1E1233 ** 2;
        a = -1E1233 ** 2;
        a = 1E1233 ** -2;
        a = -1E1233 ** -2;
        a = 1E1233 ** 1E1233;
        a = 1E1233 ** -1E1233;
        a = -1E1233 ** 1E1233;
        a = -1E1233 ** -1E1233;
    }
}
// ----
// TypeError 2271: (71-112): Built-in binary operator ** cannot be applied to types int_const 1797...(301 digits omitted)...7216 and int_const 4.
// TypeError 7407: (71-112): Type int_const 1797...(301 digits omitted)...7216 is not implicitly convertible to expected type int256. Literal is too large to fit in int256.
// TypeError 2271: (135-151): Built-in binary operator ** cannot be applied to types int_const 4 and int_const 1157...(70 digits omitted)...9936.
// TypeError 7407: (126-169): Type int_const 1340...(147 digits omitted)...4096 is not implicitly convertible to expected type int256. Literal is too large to fit in int256.
// TypeError 2271: (201-217): Built-in binary operator ** cannot be applied to types int_const 4 and int_const 1340...(147 digits omitted)...4096.
// TypeError 2271: (183-219): Built-in binary operator ** cannot be applied to types int_const 4 and int_const -115...(71 digits omitted)...9936.
// TypeError 2271: (233-244): Built-in binary operator ** cannot be applied to types int_const 2 and int_const 1000...(1226 digits omitted)...0000.
// TypeError 2271: (258-270): Built-in binary operator ** cannot be applied to types int_const -2 and int_const 1000...(1226 digits omitted)...0000.
// TypeError 2271: (284-296): Built-in binary operator ** cannot be applied to types int_const 2 and int_const -100...(1227 digits omitted)...0000.
// TypeError 2271: (310-323): Built-in binary operator ** cannot be applied to types int_const -2 and int_const -100...(1227 digits omitted)...0000.
// TypeError 2271: (337-348): Built-in binary operator ** cannot be applied to types int_const 1000...(1226 digits omitted)...0000 and int_const 2.
// TypeError 7407: (337-348): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256. Literal is too large to fit in int256.
// TypeError 2271: (362-374): Built-in binary operator ** cannot be applied to types int_const -100...(1227 digits omitted)...0000 and int_const 2.
// TypeError 7407: (362-374): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256. Literal is too large to fit in int256.
// TypeError 2271: (388-400): Built-in binary operator ** cannot be applied to types int_const 1000...(1226 digits omitted)...0000 and int_const -2.
// TypeError 7407: (388-400): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256. Literal is too large to fit in int256.
// TypeError 2271: (414-427): Built-in binary operator ** cannot be applied to types int_const -100...(1227 digits omitted)...0000 and int_const -2.
// TypeError 7407: (414-427): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256. Literal is too large to fit in int256.
// TypeError 2271: (441-457): Built-in binary operator ** cannot be applied to types int_const 1000...(1226 digits omitted)...0000 and int_const 1000...(1226 digits omitted)...0000.
// TypeError 7407: (441-457): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256. Literal is too large to fit in int256.
// TypeError 2271: (471-488): Built-in binary operator ** cannot be applied to types int_const 1000...(1226 digits omitted)...0000 and int_const -100...(1227 digits omitted)...0000.
// TypeError 7407: (471-488): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256. Literal is too large to fit in int256.
// TypeError 2271: (502-519): Built-in binary operator ** cannot be applied to types int_const -100...(1227 digits omitted)...0000 and int_const 1000...(1226 digits omitted)...0000.
// TypeError 7407: (502-519): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256. Literal is too large to fit in int256.
// TypeError 2271: (533-551): Built-in binary operator ** cannot be applied to types int_const -100...(1227 digits omitted)...0000 and int_const -100...(1227 digits omitted)...0000.
// TypeError 7407: (533-551): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256. Literal is too large to fit in int256.
