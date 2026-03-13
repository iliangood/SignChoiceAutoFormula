const std = @import("std");
const math = std.math;

const Number = struct {
    numerator: i64,
    denominator: u64,
    pub fn make(numerator: i64, denominator: u64) Number {
        return Number{
            .numerator = numerator,
            .denominator = denominator,
        };
    }
    pub fn make_simplify(numerator: i64, denominator: u64) Number {
        var res = make(numerator, denominator);
        res.simplify_inplace();
        return res;
    }
    pub fn isValid(self: *const Number) bool {
        return self.denominator != 0;
    }
    pub fn simplify_inplace(self: *Number) void {
        const greatest_common_diviser = math.gcd(@as(u64, @intCast(@abs(self.numerator))), self.denominator);
        self.numerator = @divExact(self.numerator, @as(i64, @intCast(greatest_common_diviser)));
        self.denominator /= greatest_common_diviser;
    }
    pub fn simplify(self_: Number) Number {
        var self = self_;
        self.simplify_inplace();
        return self;
    }
    pub fn add_inplace(self: *Number, other: *const Number) void {
        self.numerator *= @intCast(other.denominator);
        self.numerator += other.numerator * @as(i64, @intCast(self.denominator));
        self.denominator *= other.denominator;
        self.simplify_inplace();
    }
    pub fn add(self_: Number, other: *const Number) Number {
        var self = self_;
        self.add_inplace(other);
        return self;
    }
    pub fn sub_inplace(self: *Number, other: *const Number) void {
        self.numerator *= @intCast(other.denominator);
        self.numerator -= other.numerator * @as(i64, @intCast(self.denominator));
        self.denominator *= other.denominator;
        self.simplify_inplace();
    }
    pub fn sub(self_: Number, other: *const Number) Number {
        var self = self_;
        self.sub_inplace(other);
        return self;
    }
    pub fn mul_inplace(self: *Number, other: *const Number) void {
        self.numerator *= other.numerator;
        self.denominator *= other.denominator;
        self.simplify_inplace();
    }
    pub fn mul(self_: Number, other: *const Number) Number {
        var self = self_;
        self.mul_inplace(other);
        return self;
    }
    pub fn div_inplace(self: *Number, other: *const Number) void {
        if (other.numerator < 0) {
            self.numerator *= -@as(i64, @intCast(other.denominator));
            self.denominator *= @as(u64, @intCast(-other.numerator));
        } else {
            self.numerator *= @as(i64, @intCast(other.denominator));
            self.denominator *= @as(u64, @intCast(other.numerator));
        }
        self.simplify_inplace();
    }
    pub fn div(self_: Number, other: *const Number) Number {
        var self = self_;
        self.div_inplace(other);
        return self;
    }
    pub fn cmp(a: *const Number, b: *const Number) math.Order {
        return math.order(a.numerator * @as(i64, @intCast(b.denominator)), b.numerator * @as(i64, @intCast(a.denominator)));
    }
    // pub fn format(
    //     self: Number,
    //     comptime fmt: []const u8,
    //     options: std.fmt.FormatOptions,
    //     writer: *std.Io.writer,
    // ) std.Io.Writer.Error!void {}
    pub fn try_standart_print(self: *const Number, writer: *std.Io.Writer) std.Io.Writer.Error!void {
        try writer.print("{}/{}", .{ self.numerator, self.denominator });
    }
    // pub fn try_to_decimal(self: *const Number, allocator: std.mem.Allocator) !?[:0]u8 {
    //     if (self.denominator == 1) {
    //         std.fmt.allocPrintSentinel(allocator, "{}", .{num.numerator});
    //     }
    //     var num = self.*;
    //     const is_negative = num.numerator < 0;
    //     num.numerator = @abs(num.numerator);
    //     const twos = countMultiplier(&num.denominator, 2);
    //     const fives = countMultiplier(&num.denominator, 5);
    //     if (num.denominator != 1) {
    //         return null;
    //     }
    //     const decimal_places = @max(twos, fives);
    //     std.fmt.allocPrintSentinel(allocator, "{}")
    // }
};

fn countMultiplier(num: *u64, multiplier: u64) u64 {
    var res = 0;
    while (num.* % 2 == 0) : (num.* /= multiplier) {
        res += 1;
    }
    return res;
}

fn isDecimal(num_: u64) bool {
    var num = num_;
    if (num == 0) {
        unreachable; // Предполагается использование для знаменателя, который != 0
    }
    countMultiplier(&num, 5);
    return num & (num - 1) == 0;
}

const testing = std.testing;

test "make creates correct Number" {
    const n = Number.make(3, 4);

    try testing.expectEqual(@as(i64, 3), n.numerator);
    try testing.expectEqual(@as(u64, 4), n.denominator);
}

test "isValid works" {
    var n = Number.make(1, 2);
    try testing.expect(n.isValid());

    n.denominator = 0;
    try testing.expect(!n.isValid());
}

test "simplify reduces fraction" {
    var n = Number.make(4, 8);

    n.simplify_inplace();

    try testing.expectEqual(@as(i64, 1), n.numerator);
    try testing.expectEqual(@as(u64, 2), n.denominator);
}

test "simplify keeps already simplified fraction" {
    var n = Number.make(3, 5);

    n.simplify_inplace();

    try testing.expectEqual(@as(i64, 3), n.numerator);
    try testing.expectEqual(@as(u64, 5), n.denominator);
}

test "simplify handles negative numerator" {
    var n = Number.make(-6, 9);

    n.simplify_inplace();

    try testing.expectEqual(@as(i64, -2), n.numerator);
    try testing.expectEqual(@as(u64, 3), n.denominator);
}

test "add_inplace basic addition" {
    var a = Number.make(1, 2);
    const b = Number.make(1, 3);

    a.add_inplace(&b);

    try testing.expectEqual(@as(i64, 5), a.numerator);
    try testing.expectEqual(@as(u64, 6), a.denominator);
}

test "add works (non inplace)" {
    const a = Number.make(1, 2);
    const b = Number.make(1, 4);

    const result = a.add(&b);

    try testing.expectEqual(@as(i64, 3), result.numerator);
    try testing.expectEqual(@as(u64, 4), result.denominator);
}

test "sub_inplace basic subtraction" {
    var a = Number.make(3, 4);
    const b = Number.make(1, 4);

    a.sub_inplace(&b);

    try testing.expectEqual(@as(i64, 1), a.numerator);
    try testing.expectEqual(@as(u64, 2), a.denominator);
}

test "sub works (non inplace)" {
    const a = Number.make(5, 6);
    const b = Number.make(1, 6);

    const result = a.sub(&b);

    try testing.expectEqual(@as(i64, 2), result.numerator);
    try testing.expectEqual(@as(u64, 3), result.denominator);
}

test "mul_inplace basic multiplication" {
    var a = Number.make(2, 3);
    const b = Number.make(3, 5);

    a.mul_inplace(&b);

    try testing.expectEqual(@as(i64, 2), a.numerator);
    try testing.expectEqual(@as(u64, 5), a.denominator);
}

test "mul works (non inplace)" {
    const a = Number.make(4, 7);
    const b = Number.make(2, 3);

    const result = a.mul(&b);

    try testing.expectEqual(@as(i64, 8), result.numerator);
    try testing.expectEqual(@as(u64, 21), result.denominator);
}

test "div_inplace basic division" {
    var a = Number.make(3, 4);
    const b = Number.make(2, 5);

    a.div_inplace(&b);

    try testing.expectEqual(@as(i64, 15), a.numerator);
    try testing.expectEqual(@as(u64, 8), a.denominator);
}

test "div works (non inplace)" {
    const a = Number.make(7, 8);
    const b = Number.make(2, 3);

    const result = a.div(&b);

    try testing.expectEqual(@as(i64, 21), result.numerator);
    try testing.expectEqual(@as(u64, 16), result.denominator);
}

test "operations with negative Numbers" {
    var a = Number.make(-1, 2);
    const b = Number.make(1, 3);

    a.add_inplace(&b);

    try testing.expectEqual(@as(i64, -1), a.numerator);
    try testing.expectEqual(@as(u64, 6), a.denominator);
}

test "zero numerator multiplication" {
    var a = Number.make(0, 5);
    const b = Number.make(7, 8);

    a.mul_inplace(&b);

    try testing.expectEqual(@as(i64, 0), a.numerator);
    try testing.expectEqual(@as(u64, 1), a.denominator);
}

test "large Numbers simplify correctly" {
    var a = Number.make(1000000, 2000000);

    a.simplify_inplace();

    try testing.expectEqual(@as(i64, 1), a.numerator);
    try testing.expectEqual(@as(u64, 2), a.denominator);
}

test "cmp basic test 1" {
    const a = Number.make(8, 6);
    const b = Number.make(7, 5);
    try testing.expectEqual(math.Order.lt, a.cmp(&b));
}

test "cmp basic test 2" {
    const a = Number.make(1, 2);
    const b = Number.make(1, 2);
    try testing.expectEqual(math.Order.eq, a.cmp(&b));
}

test "cmp basic test 3" {
    const a = Number.make(7, 6);
    const b = Number.make(6, 7);
    try testing.expectEqual(math.Order.gt, a.cmp(&b));
}
