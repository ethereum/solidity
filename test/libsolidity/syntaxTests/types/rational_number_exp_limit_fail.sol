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
// TypeError: (71-102): Operator ** not compatible with types int_const 1797...(301 digits omitted)...7216 and int_const 4. Precision is limited to 4096 bits
// TypeError: (71-102): Type int_const 1797...(301 digits omitted)...7216 is not implicitly convertible to expected type int256.
// TypeError: (116-148): Operator ** not compatible with types int_const 1797...(301 digits omitted)...7216 and int_const 4. Precision is limited to 4096 bits
// TypeError: (116-153): Operator ** not compatible with types int_const 1797...(301 digits omitted)...7216 and int_const 4. Precision is limited to 4096 bits
// TypeError: (116-153): Type int_const 1797...(301 digits omitted)...7216 is not implicitly convertible to expected type int256.
// TypeError: (167-203): Operator ** not compatible with types int_const 4 and int_const -179...(302 digits omitted)...7216
// TypeError: (217-228): Operator ** not compatible with types int_const 2 and int_const 1000...(1226 digits omitted)...0000
// TypeError: (242-254): Operator ** not compatible with types int_const -2 and int_const 1000...(1226 digits omitted)...0000
// TypeError: (268-280): Operator ** not compatible with types int_const 2 and int_const -100...(1227 digits omitted)...0000
// TypeError: (294-307): Operator ** not compatible with types int_const -2 and int_const -100...(1227 digits omitted)...0000
// TypeError: (321-332): Operator ** not compatible with types int_const 1000...(1226 digits omitted)...0000 and int_const 2. Precision is limited to 4096 bits
// TypeError: (321-332): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError: (346-358): Operator ** not compatible with types int_const -100...(1227 digits omitted)...0000 and int_const 2. Precision is limited to 4096 bits
// TypeError: (346-358): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError: (372-384): Operator ** not compatible with types int_const 1000...(1226 digits omitted)...0000 and int_const -2. Precision is limited to 4096 bits
// TypeError: (372-384): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError: (398-411): Operator ** not compatible with types int_const -100...(1227 digits omitted)...0000 and int_const -2. Precision is limited to 4096 bits
// TypeError: (398-411): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError: (425-441): Operator ** not compatible with types int_const 1000...(1226 digits omitted)...0000 and int_const 1000...(1226 digits omitted)...0000
// TypeError: (425-441): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError: (455-472): Operator ** not compatible with types int_const 1000...(1226 digits omitted)...0000 and int_const -100...(1227 digits omitted)...0000
// TypeError: (455-472): Type int_const 1000...(1226 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError: (486-503): Operator ** not compatible with types int_const -100...(1227 digits omitted)...0000 and int_const 1000...(1226 digits omitted)...0000
// TypeError: (486-503): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256.
// TypeError: (517-535): Operator ** not compatible with types int_const -100...(1227 digits omitted)...0000 and int_const -100...(1227 digits omitted)...0000
// TypeError: (517-535): Type int_const -100...(1227 digits omitted)...0000 is not implicitly convertible to expected type int256.
