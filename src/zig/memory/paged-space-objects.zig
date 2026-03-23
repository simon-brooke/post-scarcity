/// Header for objects which are allocated in pages.
const PagedSpaceObjectHeader = struct {
    const tag = union {
        const bytes = [4]u8;
        const value = u32;
    };
    var count = u32;
    const acl = u64; // later when we have a pointer object defined this will be substituted
};

const PSO4: type = struct {
    const PagedSpaceObjectHeader: header;
    const payload = union {
        var bytes: [8]u8;
        var words: [2]u64;
    };
};
