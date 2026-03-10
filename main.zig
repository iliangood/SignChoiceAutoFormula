const std = @import("std");
const c = @cImport({
    @cInclude("fractions.h");
    @cInclude("malloc.h");
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

fn formula_small(nums: []const c.number) !?[*:0]u8 {
    return switch (nums.len) {
        0 => null,
        1 => @as(?[*:0]u8, c.ntoa(nums[0])) orelse return error.OutOfMemory,
        else => unreachable,
    };
}

fn formula_mut(allocator: std.mem.Allocator, nums: []c.number) !?[*:0]u8 {
    if (nums.len < 2) {
        return try formula_small(nums);
    }
    if (nums.len == 2) {
        const divider = c.maken(2, 1);
        const num1 = @as(?[*:0]u8, c.ntoa(c.divn(c.addn(nums[0], nums[1]), divider))) orelse return error.OutOfMemory;
        defer allocator.free(std.mem.span(num1));
        const num2 = @as(?[*:0]u8, c.ntoa(c.absn(c.divn(c.subn(nums[1], nums[0]), divider)))) orelse return error.OutOfMemory;
        defer allocator.free(std.mem.span(num2));
        return try std.fmt.allocPrintSentinel(allocator, "{s} ± {s}", .{ num1, num2 }, 0);
    }
    const last_num = nums[nums.len - 1];
    for (nums) |*num|
        num.* = c.subn(num.*, last_num);
    const recurse = (try formula_mut(allocator, nums[0 .. nums.len - 1])).?;
    defer allocator.free(std.mem.span(recurse));
    const num = @as(?[*:0]u8, c.ntoa(c.absn(last_num))) orelse return error.OutOfMemory;
    defer allocator.free(std.mem.span(num));
    const sign: u8 = if (c.isNegative(last_num)) '-' else '+';
    return try std.fmt.allocPrintSentinel(allocator, "(0.5 ± 0.5)*({s}){c}{s}", .{
        recurse,
        sign,
        num,
    }, 0);
}

fn formula(nums: []const c.number, allocator: std.mem.Allocator) !?[*:0]u8 {
    if (nums.len < 2) {
        return try formula_small(nums);
    }
    const mut_nums = try allocator.dupe(c.number, nums);
    defer allocator.free(mut_nums);
    return formula_mut(allocator, mut_nums);
}

fn test_formula_mut(allocator: std.mem.Allocator, expected: [:0]const u8, nums: []c.number) !void {
    const testing = std.testing;
    const result = try formula_mut(allocator, nums) orelse return error.TestUnexpectedNull;
    defer allocator.free(std.mem.span(result));
    try testing.expectEqualStrings(expected, std.mem.span(result));
}

test "formula function" {
    const testing = std.testing;
    // var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    // var allocator = gpa.allocator(); // Временно не используется, так как в c коде используется mallo
    const allocator = std.heap.c_allocator;
    {
        var arr = [_]c.number{};
        try testing.expectEqual(null, try formula_mut(allocator, &arr));
    }
    {
        var arr = [_]c.number{c.maken(2, 3)};
        try test_formula_mut(allocator, "2/3", &arr);
    }
    {
        var arr = [_]c.number{c.maken(3, 2)};
        try test_formula_mut(allocator, "1.5", &arr);
    }
    {
        var arr = [_]c.number{
            c.maken(1, 1),
            c.maken(2, 1),
        };
        try test_formula_mut(allocator, "1.5 ± 0.5", &arr);
    }
    {
        var arr = [_]c.number{
            c.maken(1, 1),
            c.maken(2, 1),
            c.maken(3, 1),
        };
        try test_formula_mut(allocator, "(0.5 ± 0.5)*(-1.5 ± 0.5)+3", &arr);
    }
}

fn num_less(_: void, a: c.number, b: c.number) bool {
    return c.cmpn(a, b) == -1;
}
fn num_eq(a: c.number, b: c.number) bool {
    return c.cmpn(a,b) == 0;
}

fn remove_duplicates(T: type, nums: []T, comptime lessThenFn: fn (void, T, T) bool, equalFn: fn (T, T) bool) []T {
    if (nums.len < 2)
        return nums;
    std.mem.sort(T, nums, {}, lessThenFn);
    var i: usize = 0;
    for (nums[1..]) |*num| {
        if (!equalFn(num.*, nums[i])) {
            i += 1;
            nums[i] = num.*;
        }
    }
    return nums[0 .. i + 1];
}

fn lessThan_u8(_: void, lhs: u8, rhs: u8) bool {
    return lhs < rhs;
}
fn eq_u8(a: u8, b: u8) bool {
    return a == b;
}

test "remove_duplicates - basic" {
    const testing = std.testing;
    {
        var arr = [_]u8{ 1, 1, 43, 34, 43 };
        const expected = [_]u8{ 1, 34, 43 };
        try testing.expectEqualSlices(u8, &expected, remove_duplicates(u8, &arr, lessThan_u8, eq_u8));
    }
}

test "remove_duplicates - empty and single element" {
    const testing = std.testing;

    var empty = [_]u8{};
    try testing.expectEqualSlices(u8, &[_]u8{}, remove_duplicates(u8, &empty, lessThan_u8, eq_u8));

    var single = [_]u8{5};
    try testing.expectEqualSlices(u8, &[_]u8{5}, remove_duplicates(u8, &single, lessThan_u8, eq_u8));
}

test "remove_duplicates - all identical elements" {
    const testing = std.testing;

    var identical = [_]u8{ 10, 10, 10, 10 };
    const expected = [_]u8{10};
    try testing.expectEqualSlices(u8, &expected, remove_duplicates(u8, &identical, lessThan_u8, eq_u8));
}

test "remove_duplicates - already sorted and unique" {
    const testing = std.testing;

    var sorted = [_]u8{ 1, 2, 3, 4, 5 };
    const expected = [_]u8{ 1, 2, 3, 4, 5 };
    try testing.expectEqualSlices(u8, &expected, remove_duplicates(u8, &sorted, lessThan_u8, eq_u8));
}

test "remove_duplicates - large range and unsorted" {
    const testing = std.testing;

    var arr = [_]u8{ 100, 2, 100, 2, 1, 50, 50, 1 };
    const expected = [_]u8{ 1, 2, 50, 100 };
    try testing.expectEqualSlices(u8, &expected, remove_duplicates(u8, &arr, lessThan_u8, eq_u8));
}

pub fn main(init: std.process.Init) !void {
    // var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    // const allocator = gpa.allocator();
    // defer _ = gpa.deinit();
    const allocator = std.heap.c_allocator;
    const io = init.io;
    var stdout_writer = std.Io.File.stdout().writer(io, &.{});
    const stdout = &stdout_writer.interface;

    var args = try init.minimal.args.toSlice(allocator);
    if (args.len == 1) {
        try stdout.print("enter at least 1 number.\n example:\n ./main 1 2 3\n", .{});
        return;
    }
    const nums = try allocator.alloc(c.number, args.len-1);
    for (args[1..], nums) |arg, *num| {
        num.* =  c.aton(arg);
    }

    try stdout.print("{s}", .{(try formula_mut(allocator, remove_duplicates(c.number, nums, num_less, num_eq))).?});
    // std.debug.print("{s}\n", .{(try formula(&arr1, allocator)).?});
    // std.debug.print("{}\n", .{getCharsN(u32, 17)});
}
