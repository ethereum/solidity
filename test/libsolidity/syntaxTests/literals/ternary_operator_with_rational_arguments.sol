contract C {
    function f(bool cond) public {
         // OK
         int8 v1 = cond ? -1 : 1;
         int16 v2 = cond ? -32768 : 1;
         int16 v3 = cond ? -32768 : -1;
         int16 v4 = cond ? -32768 : int8(1);
         int16 v5 = cond ? int16(1) : 1;
         uint16 v6 = cond ? uint16(1) : 1;
         ufixed8x1 v7 = cond ? 1.0 : 1.1;
         ufixed8x1 v8 = cond ? 0 : 1.1;
         fixed8x1 v9 = cond ? -1.0 : -1.1;
         fixed8x1 v10 = cond ? -1.1 : 1;

         // Errors
         cond ? 32768 : -1;
         cond ? int16(-1) : uint8(1);
         cond ? int8(-1) : uint8(1);
         cond ? -1 : 1.1;
         cond ? -1.0 : 1.1;
         cond ? true : 1;
    }
}
// ----
// TypeError 1080: (500-517): True expression's type uint16 does not match false expression's type int8.
// TypeError 1080: (528-555): True expression's type int16 does not match false expression's type uint8.
// TypeError 1080: (566-592): True expression's type int8 does not match false expression's type uint8.
// TypeError 1080: (603-618): True expression's type int8 does not match false expression's type ufixed8x1.
// TypeError 1080: (629-646): True expression's type int8 does not match false expression's type ufixed8x1.
// TypeError 1080: (657-672): True expression's type bool does not match false expression's type uint8.
