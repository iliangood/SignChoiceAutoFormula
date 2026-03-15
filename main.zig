const std = @import("std");
const config = @import("config");
const Number = @import("fractions").Number;

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

fn formula_small(allocator: std.mem.Allocator, nums: []const Number) !?[:0]u8 {
    return switch (nums.len) {
        0 => null,
        1 => try std.fmt.allocPrintSentinel(allocator, "{f}", .{nums[0]}, 0),
        else => unreachable,
    };
}

fn formula_mut(allocator: std.mem.Allocator, nums: []Number) !?[:0]u8 {
    if (nums.len < 2) {
        return try formula_small(allocator, nums);
    }
    if (nums.len == 2) {
        const divider = Number.make(2, 1);
        const num1 = nums[0].add(&nums[1]).div(&divider);
        const num2 = (nums[1].sub(&nums[0])).div(&divider);
        return try std.fmt.allocPrintSentinel(allocator, "{f} ± {f}", .{ num1, num2 }, 0);
    }
    const last_num = nums[nums.len - 1];
    for (nums) |*num|
        num.sub_inplace(&last_num);
    const recurse = (try formula_mut(allocator, nums[0 .. nums.len - 1])).?;
    defer allocator.free(recurse);
    const num = last_num.abs();
    const sign: u8 = if (last_num.is_negative()) '-' else '+';
    // _ = num;
    return try std.fmt.allocPrintSentinel(allocator, "(0.5 ± 0.5)*({s}){c}{f}", .{
        recurse,
        sign,
        num,
    }, 0);
}

fn formula(nums: []const Number, allocator: std.mem.Allocator) !?[*:0]u8 {
    if (nums.len < 2) {
        return try formula_small(allocator, nums);
    }
    const mut_nums = try allocator.dupe(Number, nums);
    defer allocator.free(mut_nums);
    return formula_mut(allocator, mut_nums);
}

fn test_formula_mut(allocator: std.mem.Allocator, expected: [:0]const u8, nums: []Number) !void {
    const testing = std.testing;
    const result = try formula_mut(allocator, nums) orelse return error.TestUnexpectedNull;
    defer allocator.free(result);
    try testing.expectEqualStrings(expected, result);
}

test "formula function" {
    const testing = std.testing;
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};

    const allocator = gpa.allocator(); // Временно не используется, так как в c коде используется mallo
    // const allocator = std.heap.c_allocator;
    {
        var arr = [_]Number{};
        try testing.expectEqual(null, try formula_mut(allocator, &arr));
    }
    {
        var arr = [_]Number{Number.make(2, 3)};
        try test_formula_mut(allocator, "2/3", &arr);
    }
    {
        var arr = [_]Number{Number.make(3, 2)};
        try test_formula_mut(allocator, "1.5", &arr);
    }
    {
        var arr = [_]Number{
            Number.make(1, 1),
            Number.make(2, 1),
        };
        try test_formula_mut(allocator, "1.5 ± 0.5", &arr);
    }
    {
        var arr = [_]Number{
            Number.make(1, 1),
            Number.make(2, 1),
            Number.make(3, 1),
        };
        try test_formula_mut(allocator, "(0.5 ± 0.5)*(-1.5 ± 0.5)+3", &arr);
    }
}

fn num_less(_: void, a: Number, b: Number) bool {
    return a.cmp(&b) == .lt;
}
fn num_eq(a: Number, b: Number) bool {
    return a.cmp(&b) == .eq;
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
    var buf: [262144]u8 = undefined;

    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const gpa_allocator = gpa.allocator();
    defer _ = gpa.deinit();
    // var arena = std.heap.ArenaAllocator.init(gpa_allocator);
    // arena.allocator
    // defer arena.deinit();
    // const allocator = arena.allocator();
    // const allocator = std.heap.c_allocator;
    // var
    var fallbackAllocator = std.heap.StackFallbackAllocator(262144){
        .buffer = buf,
        .fallback_allocator = gpa_allocator,
        .fixed_buffer_allocator = std.heap.FixedBufferAllocator.init(&buf),
    };
    const allocator = fallbackAllocator.get();

    const io = init.io;
    var stdout_writer = std.Io.File.stdout().writer(io, &.{});
    const stdout = &stdout_writer.interface;
    const start = std.Io.Clock.real.now(io);
    var args = try init.minimal.args.toSlice(allocator);
    if (args.len == 1) {
        try stdout.print("enter at least 1 number.\n example:\n ./main 1 2 3\n", .{});
        return;
    }
    const nums = try allocator.alloc(Number, args.len - 1);
    for (args[1..], nums) |arg, *num| {
        num.* = Number.parse(arg) catch |err| {
            switch (err) {
                Number.ParseError.FormatError, Number.ParseError.InvalidCharacter => try stdout.print("Invalid format of number: \"{s}\"", .{arg}),
                Number.ParseError.IsEmpty => try stdout.print("Empty number", .{}),
                Number.ParseError.Overflow, Number.ParseError.Underflow => try stdout.print("num is too long", .{}),
            }
            return;
        };
    }
    const mid = std.Io.Clock.real.now(io);
    const res = (try formula_mut(allocator, remove_duplicates(Number, nums, num_less, num_eq))).?;
    defer allocator.free(res);
    const end = std.Io.Clock.real.now(io);
    if (config.measure_time) {
        const duration = std.Io.Timestamp.durationTo(start, end);
        const dur1 = std.Io.Timestamp.durationTo(start, mid);
        const dur2 = std.Io.Timestamp.durationTo(mid, end);
        try stdout.print("elapsed:{f}\nparse:{f}\nformula:{f}\n", .{ duration, dur1, dur2 });
    }

    try stdout.print("{s}\n", .{res});
}
