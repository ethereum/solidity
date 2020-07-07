contract c {
    function f() public pure {
        int a;
        a = 4 ** 4 ** 2 ** 4 ** 4 ** 4 ** 4;
        a = -4 ** 4 ** 2 ** 4 ** 4 ** 4 ** 4 ** 4;
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
// TypeError 2271: (71-102): Operator ** not compatible with types int_const 1797...(301 digits omitted)...7216 and int_const 4. Precision of rational constants is limited to 4096 bits.
// TypeError 7407: (71-102): Type int_const 1797...(301 digits omitted)...7216 is not implicitly convertible to expected type int256.
// TypeError 2271: (116-148): Operator ** not compatible with types int_const 1797...(301 digits omitted)...7216 and int_const 4. Precision of rational constants is limited to 4096 bits.
// TypeError 2271: (116-153): Operator ** not compatible with types int_const 1797...(301 digits omitted)...7216 and int_const 4. Precision of rational constants is limited to 4096 bits.
// TypeError 7407: (116-153): Type int_const 1797...(301 digits omitted)...7216 is not implicitly convertible to expected type int256.
// TypeError 2271: (167-203): Operator ** not compatible with types int_const 4 and int_const -179...(302 digits omitted)...7216
// TypeError 2271: (217-228): Operator ** not compatible with types int_const 2 and int_const 1000...(1226 digits omitted)...0000
// TypeError 2271: (242-254): Operator ** not compatible with types int_const -2 and int_const 1000...(1226 digits omitted)...0000
// TypeError 2271: (268-280): Operator ** not compatible with types int_const 2 and int_const -100...(1227 digits omitted)...0000
// TypeError 2271: (294-307): Operator ** not compatible with types int_const -2 and int_const -100...(1227 digits omitted)...0000
// TypeError 2271: (321-332): Operator ** not compatible with types int_const 1000...(1226 digits omitted)...0000 and int_const 2. Precision of rational constants is limited to 4096 bits.
// TypeError 7407: (321-332): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError 2271: (346-358): Operator ** not compatible with types int_const -100...(1227 digits omitted)...0000 and int_const 2. Precision of rational constants is limited to 4096 bits.
// TypeError 7407: (346-358): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError 2271: (372-384): Operator ** not compatible with types int_const 1000...(1226 digits omitted)...0000 and int_const -2. Precision of rational constants is limited to 4096 bits.
// TypeError 7407: (372-384): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError 2271: (398-411): Operator ** not compatible with types int_const -100...(1227 digits omitted)...0000 and int_const -2. Precision of rational constants is limited to 4096 bits.
// TypeError 7407: (398-411): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError 2271: (425-441): Operator ** not compatible with types int_const 1000...(1226 digits omitted)...0000 and int_const 1000...(1226 digits omitted)...0000
// TypeError 7407: (425-441): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError 2271: (455-472): Operator ** not compatible with types int_const 1000...(1226 digits omitted)...0000 and int_const -100...(1227 digits omitted)...0000
// TypeError 7407: (455-472): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError 2271: (486-503): Operator ** not compatible with types int_const -100...(1227 digits omitted)...0000 and int_const 1000...(1226 digits omitted)...0000
// TypeError 7407: (486-503): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError 2271: (517-535): Operator ** not compatible with types int_const -100...(1227 digits omitted)...0000 and int_const -100...(1227 digits omitted)...0000
// TypeError 7407: (517-535): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256.
