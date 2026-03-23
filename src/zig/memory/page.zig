/// A Page is an area of memory in which objects are stored. Every page has
/// a header, and every page header has common structure. The objects stored
/// on any page are all PagedObjects, q.v. and, on any given page, all the
/// objects stored on that page are of the same size.
const Page = struct {
    const content = union {
        const bytes = [1048576]u8;
    };
};
