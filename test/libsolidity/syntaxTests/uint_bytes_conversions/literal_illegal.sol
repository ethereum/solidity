contract C {
    bytes2 hex1 = 0xAABBCC;
    bytes2 hex2 = hex"AABBCC";
    bytes2 text = "123456";
}
// ----
// TypeError: (31-39): Type int_const 11189196 is not implicitly convertible to expected type bytes2.
// TypeError: (59-70): Type literal_string (contains invalid UTF-8 sequence at position 0) is not implicitly convertible to expected type bytes2.
// TypeError: (90-98): Type literal_string "123456" is not implicitly convertible to expected type bytes2.
