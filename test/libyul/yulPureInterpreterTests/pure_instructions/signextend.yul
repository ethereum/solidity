{

    function check(a, b)
    { if iszero(eq(a, b)) { revert(0, 0) } }

    check(signextend(0x0, 0x0), 0x0)
    check(signextend(0x0, 0x1), 0x1)
    check(signextend(0x0, 0x2), 0x2)
    check(signextend(0x0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(signextend(0x0, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(signextend(0x0, 0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffb4)
    check(signextend(0x0, 0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8a)
    check(signextend(0x0, 0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffb5)
    check(signextend(0x1, 0x0), 0x0)
    check(signextend(0x1, 0x1), 0x1)
    check(signextend(0x1, 0x2), 0x2)
    check(signextend(0x1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(signextend(0x1, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(signextend(0x1, 0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4), 0x47b4)
    check(signextend(0x1, 0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a), 0x308a)
    check(signextend(0x1, 0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbdb5)
    check(signextend(0x2, 0x0), 0x0)
    check(signextend(0x2, 0x1), 0x1)
    check(signextend(0x2, 0x2), 0x2)
    check(signextend(0x2, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(signextend(0x2, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(signextend(0x2, 0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffff347b4)
    check(signextend(0x2, 0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc2308a)
    check(signextend(0x2, 0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5), 0x6bdb5)
    check(signextend(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x0), 0x0)
    check(signextend(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x1), 0x1)
    check(signextend(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x2), 0x2)
    check(signextend(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(signextend(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(signextend(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4), 0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4)
    check(signextend(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a), 0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a)
    check(signextend(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5), 0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5)
    check(signextend(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x0), 0x0)
    check(signextend(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x1), 0x1)
    check(signextend(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x2), 0x2)
    check(signextend(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(signextend(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(signextend(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4), 0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4)
    check(signextend(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a), 0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a)
    check(signextend(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5), 0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5)
    check(signextend(0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4, 0x0), 0x0)
    check(signextend(0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4, 0x1), 0x1)
    check(signextend(0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4, 0x2), 0x2)
    check(signextend(0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(signextend(0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(signextend(0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4, 0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4), 0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4)
    check(signextend(0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4, 0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a), 0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a)
    check(signextend(0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4, 0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5), 0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5)
    check(signextend(0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a, 0x0), 0x0)
    check(signextend(0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a, 0x1), 0x1)
    check(signextend(0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a, 0x2), 0x2)
    check(signextend(0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(signextend(0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(signextend(0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a, 0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4), 0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4)
    check(signextend(0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a, 0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a), 0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a)
    check(signextend(0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a, 0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5), 0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5)
    check(signextend(0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5, 0x0), 0x0)
    check(signextend(0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5, 0x1), 0x1)
    check(signextend(0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5, 0x2), 0x2)
    check(signextend(0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(signextend(0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(signextend(0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5, 0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4), 0x6855c4590e561bd51e79061b0952b669a42287b819f7736eb72de8dc03f347b4)
    check(signextend(0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5, 0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a), 0x61138948e40817554fc48fd1ffd4c0d2a6faff424e33da38076f97530ec2308a)
    check(signextend(0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5, 0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5), 0x36b4781b870310da5481b23fe98b33115c6e598ebc74621803b9bc766b06bdb5)
}
// ====
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
