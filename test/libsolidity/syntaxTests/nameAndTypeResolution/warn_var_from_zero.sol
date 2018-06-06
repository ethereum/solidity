 contract test {
     function f() pure public {
         for (var i = 0; i < msg.data.length; i++) { }
     }
 }
// ----
// Warning: (63-68): Use of the "var" keyword is deprecated.
// Warning: (63-72): The type of this variable was inferred as uint8, which can hold values between 0 and 255. This is probably not desired. Use an explicit type to silence this warning.
