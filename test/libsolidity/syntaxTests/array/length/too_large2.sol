contract C { bytes[8**80][65536] i; }
// ----
// TypeError: (13-34): Array too large for storage.
