contract c {
	mapping(string => uint) public data;
}
// ----
// TypeError: (14-49): Dynamically-sized keys for public mappings are not supported.
