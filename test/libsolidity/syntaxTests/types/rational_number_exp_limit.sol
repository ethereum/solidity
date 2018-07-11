contract c {
    function f() public pure {
        int a;
        a = 4 ** 4 ** 2 ** 4 ** 4 ** 4 ** 4;
        a = -4 ** 4 ** 2 ** 4 ** 4 ** 4 ** 4 ** 4;
        a = 4 ** (-(2 ** 4 ** 4 ** 4 ** 4 ** 4));
        a = 0 ** 1E1233; // fine
        a = 1 ** 1E1233; // fine
        a = -1 ** 1E1233; // fine
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
        a = 0E123456789; // fine
    }
}
// ----
// TypeError: (71-102): Operator ** not compatible with types int_const 1797...(301 digits omitted)...7216 and int_const 4
// TypeError: (71-102): Type int_const 1797...(301 digits omitted)...7216 is not implicitly convertible to expected type int256.
// TypeError: (116-148): Operator ** not compatible with types int_const 1797...(301 digits omitted)...7216 and int_const 4
// TypeError: (116-153): Operator ** not compatible with types int_const 1797...(301 digits omitted)...7216 and int_const 4
// TypeError: (116-153): Type int_const 1797...(301 digits omitted)...7216 is not implicitly convertible to expected type int256.
// TypeError: (167-203): Operator ** not compatible with types int_const 4 and int_const -179...(302 digits omitted)...7216
// TypeError: (317-328): Operator ** not compatible with types int_const 2 and int_const 1000...(1226 digits omitted)...0000
// TypeError: (342-354): Operator ** not compatible with types int_const -2 and int_const 1000...(1226 digits omitted)...0000
// TypeError: (368-380): Operator ** not compatible with types int_const 2 and int_const -100...(1227 digits omitted)...0000
// TypeError: (394-407): Operator ** not compatible with types int_const -2 and int_const -100...(1227 digits omitted)...0000
// TypeError: (421-432): Operator ** not compatible with types int_const 1000...(1226 digits omitted)...0000 and int_const 2
// TypeError: (421-432): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError: (446-458): Operator ** not compatible with types int_const -100...(1227 digits omitted)...0000 and int_const 2
// TypeError: (446-458): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError: (472-484): Operator ** not compatible with types int_const 1000...(1226 digits omitted)...0000 and int_const -2
// TypeError: (472-484): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError: (498-511): Operator ** not compatible with types int_const -100...(1227 digits omitted)...0000 and int_const -2
// TypeError: (498-511): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError: (525-541): Operator ** not compatible with types int_const 1000...(1226 digits omitted)...0000 and int_const 1000...(1226 digits omitted)...0000
// TypeError: (525-541): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError: (555-572): Operator ** not compatible with types int_const 1000...(1226 digits omitted)...0000 and int_const -100...(1227 digits omitted)...0000
// TypeError: (555-572): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError: (586-603): Operator ** not compatible with types int_const -100...(1227 digits omitted)...0000 and int_const 1000...(1226 digits omitted)...0000
// TypeError: (586-603): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError: (617-635): Operator ** not compatible with types int_const -100...(1227 digits omitted)...0000 and int_const -100...(1227 digits omitted)...0000
// TypeError: (617-635): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256.
