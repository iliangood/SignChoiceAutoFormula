const std = @import("std");
const c = @cImport({
    @cInclude("fractions.h");
});

// Считает сколко необходимо символов для записи числа в десятичной системе счисления
fn getCharsN(T: type, num: T) u32 {
    const hasSign = num < 0;
    if (num == 0)
        return 1;
    var number = @abs(num);
    var count: u32 = 0;
    while (number > 0) {
        number /= 10;
        count += 1;
    }
    return count + @as(u32, @intFromBool(hasSign));
}

test "getCharsN function" {
    const testing = std.testing;

    try testing.expectEqual(1, getCharsN(i32, 0));
    try testing.expectEqual(1, getCharsN(i32, 1));
    try testing.expectEqual(2, getCharsN(i32, -1));
    try testing.expectEqual(5, getCharsN(i32, 12413));
    try testing.expectEqual(6, getCharsN(i32, -12413));
}

fn formula(nums: []const c.number) ?[*:0]u8 {
    if (nums.len == 0) {
        return null;
    }
    if (nums.len == 1) {
        return c.ntoa(nums[0]);
    }
    return null;
}

test "formula function" {
    const testing = std.testing;
    const arr1 = [_]c.number{c.maken(1, 1)};
    try testing.expectEqual("1", formula(&arr1));
}

pub fn main() void {
    std.debug.print("{}\n", .{getCharsN(u32, 17)});
}
