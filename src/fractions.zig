const std = @import("std");
const math = std.math;

pub const Number = struct {
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
    pub fn is_valid(self: *const Number) bool {
        return self.denominator != 0;
    }
    pub fn is_negative(self: *const Number) bool {
        return self.numerator < 0;
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
    pub fn abs_inplace(self: *Number) void {
        self.numerator = @intCast(@abs(self.numerator));
    }
    pub fn abs(self_: *const Number) Number {
        var self = self_.*;
        self.abs_inplace();
        return self;
    }
    pub fn cmp(a: *const Number, b: *const Number) math.Order {
        return math.order(a.numerator * @as(i64, @intCast(b.denominator)), b.numerator * @as(i64, @intCast(a.denominator)));
    }
    pub fn format(self: Number, writer: *std.Io.Writer) std.Io.Writer.Error!void {
        (try self.to_decimal(writer)) orelse
            try self.mixed_print(writer);
    }

    fn standart_print(self: *const Number, writer: *std.Io.Writer) std.Io.Writer.Error!void {
        try writer.print("{}/{}", .{ self.numerator, self.denominator });
    }

    fn mixed_print(self: *const Number, writer: *std.Io.Writer) std.Io.Writer.Error!void {
        const abs_numerator = @as(u64, @intCast(@abs(self.numerator)));
        if (abs_numerator < self.denominator) {
            return self.standart_print(writer);
        }
        return writer.print("{} {}/{}", .{
            @divTrunc(self.numerator, @as(i64, @intCast(self.denominator))),
            abs_numerator % self.denominator,
            self.denominator,
        });
    }

    fn to_decimal(self: *const Number, writer: *std.Io.Writer) std.Io.Writer.Error!?void {
        if (self.denominator == 1) {
            try writer.print("{}", .{self.numerator});
            return;
        }
        var denominator = self.denominator;
        const isNegative = self.numerator < 0;
        const numerator = @as(u64, @intCast(@abs(self.numerator)));
        const twos = countMultiplier(&denominator, 2);
        const fives = countMultiplier(&denominator, 5);
        if (denominator != 1) {
            return null;
        }
        const decimal_places: u64 = @max(twos, fives);
        const whole_part = numerator / self.denominator;
        const dec_part: u64 = (numerator % self.denominator) * if (twos > fives)
            std.math.powi(u64, 5, twos - fives) catch {
                return null;
            }
        else
            std.math.powi(u64, 2, fives - twos) catch {
                return null;
            };
        try writer.print("{s}{}.{[2]:0>[3]}", .{
            if (isNegative) "-" else "",
            whole_part,
            dec_part,
            decimal_places,
        });
    }
    pub const ParseError = error{ FormatError, IsEmpty } || std.fmt.ParseIntError || error{Underflow};
    pub fn parse(str: []const u8) ParseError!Number {
        if (str.len == 0) {
            return ParseError.IsEmpty;
        }
        const isNegative = str[0] == '-';
        var i: usize = @as(usize, @intFromBool(isNegative or str[0] == '+'));
        if (i == 1 and str.len == 1) {
            return ParseError.FormatError;
        }
        if (str[i] == '.') {
            i += 1;
            const start_dec_part = i;
            while (i < str.len and std.ascii.isDigit(str[i])) : (i += 1) {}
            const end_dec_part = i;
            if (start_dec_part == end_dec_part) {
                return ParseError.FormatError; // Число не должно быть пустым
            }
            if (i != str.len) {
                return ParseError.FormatError; // Дальше ничего не должно быть
            }
            const dec_part = try std.fmt.parseUnsigned(u64, str[start_dec_part..end_dec_part], 10);
            const denominator = try std.math.powi(u64, 10, end_dec_part - start_dec_part);
            const abs_numerator = @as(i64, @intCast(dec_part));
            const numerator = if (isNegative) -abs_numerator else abs_numerator;
            return Number.make_simplify(numerator, denominator);
        }
        const start_first_num = i;
        while (i < str.len and std.ascii.isDigit(str[i])) : (i += 1) {}
        const end_first_num = i;
        if (end_first_num == start_first_num) {
            return ParseError.FormatError; // Число не должно быть пустым
        }
        const first_num = try std.fmt.parseUnsigned(u64, str[start_first_num..end_first_num], 10);
        if (end_first_num == str.len or (str[i] == '.' and end_first_num + 1 == str.len)) {
            const abs_numerator = @as(i64, @intCast(first_num));
            const numerator = if (isNegative) -abs_numerator else abs_numerator;
            return Number.make(numerator, 1);
        }
        switch (str[end_first_num]) {
            ' ' => {
                const whole_part = first_num;
                while (i < str.len and str[i] == ' ') : (i += 1) {}
                const start_numerator = i;
                while (i < str.len and std.ascii.isDigit(str[i])) : (i += 1) {}
                const end_numerator = i;
                if (i == str.len) {
                    return ParseError.FormatError; // Число не может заканчиваться ' '
                }
                if (start_numerator == end_numerator) {
                    return ParseError.FormatError; // Число не может быть нулевой длины
                }
                if (str[i] != '/') {
                    return ParseError.FormatError; // дальше должна идти дробная черта
                }
                const numerator = try std.fmt.parseUnsigned(u64, str[start_numerator..end_numerator], 10);
                i += 1;
                const start_denominator = i;
                while (i < str.len and std.ascii.isDigit(str[i])) : (i += 1) {}
                const end_denominator = i;
                if (i != str.len) {
                    return ParseError.FormatError; // Дальше ничего не должно быть
                }
                if (start_denominator == end_denominator) {
                    return ParseError.FormatError; // Число не должно быть пустым
                }
                const denominator = try std.fmt.parseUnsigned(u64, str[start_denominator..end_denominator], 10);
                const abs_res_numerator = @as(i64, @intCast(whole_part * denominator + numerator));
                const res_numerator = if (isNegative) -abs_res_numerator else abs_res_numerator;
                return Number.make_simplify(res_numerator, denominator);
            },
            '.' => {
                const whole_part = first_num;
                i += 1;
                const start_dec_part = i;
                while (i < str.len and std.ascii.isDigit(str[i])) : (i += 1) {}
                const end_dec_part = i;
                if (start_dec_part == end_dec_part) {
                    return ParseError.FormatError; // Число не должно быть пустым
                }
                if (i != str.len) {
                    return ParseError.FormatError; // Дальше ничего не должно быть
                }
                const dec_part = try std.fmt.parseUnsigned(u64, str[start_dec_part..end_dec_part], 10);
                const denominator = try std.math.powi(u64, 10, end_dec_part - start_dec_part);
                const abs_numerator = @as(i64, @intCast(dec_part + whole_part * denominator));
                const numerator = if (isNegative) -abs_numerator else abs_numerator;
                return Number.make_simplify(numerator, denominator);
            },
            '/' => {
                const abs_numerator = @as(i64, @intCast(first_num));
                i += 1;
                const start_denominator = i;
                while (i < str.len and std.ascii.isDigit(str[i])) : (i += 1) {}
                const end_denominator = i;
                if (start_denominator == end_denominator) {
                    return ParseError.FormatError; // Число не должно быть пустым
                }
                if (i != str.len) {
                    return ParseError.FormatError; // После числа ничего не должно идти
                }
                const denominator = try std.fmt.parseUnsigned(u64, str[start_denominator..end_denominator], 10);
                const numerator = if (isNegative) -abs_numerator else abs_numerator;
                return Number.make_simplify(numerator, denominator);
            },
            else => return ParseError.FormatError,
        }
        unreachable;
    }
};

fn countMultiplier(num: *u64, multiplier: u64) u64 {
    var res: u64 = 0;
    while (num.* % multiplier == 0) : (num.* /= multiplier) {
        res += 1;
    }
    return res;
}

const testing = std.testing;

test "make creates correct Number" {
    const n = Number.make(3, 4);

    try testing.expectEqual(@as(i64, 3), n.numerator);
    try testing.expectEqual(@as(u64, 4), n.denominator);
}

test "isValid works" {
    var n = Number.make(1, 2);
    try testing.expect(n.is_valid());

    n.denominator = 0;
    try testing.expect(!n.is_valid());
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

test "format basic test" {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer std.debug.assert(gpa.deinit() == .ok);
    const allocator = gpa.allocator();

    const num = Number.make(2, 3);
    const s = try std.fmt.allocPrint(allocator, "{f}", .{num});
    defer allocator.free(s);
    try testing.expectEqualStrings("2/3", s);
}

test "format decimal" {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer std.debug.assert(gpa.deinit() == .ok);

    const allocator = gpa.allocator();

    const num = Number.make(4, 5);
    const s = try std.fmt.allocPrint(allocator, "{f}", .{num});
    defer allocator.free(s);

    try testing.expectEqualStrings("0.8", s);
}
test "format: proper fraction < 1" {
    const num = Number.make(2, 5);
    const s = try std.fmt.allocPrint(testing.allocator, "{f}", .{num});
    defer testing.allocator.free(s);

    try testing.expectEqualStrings("0.4", s);
}

test "format: improper fraction → mixed number" {
    const num = Number.make(11, 4);
    const s = try std.fmt.allocPrint(testing.allocator, "{f}", .{num});
    defer testing.allocator.free(s);

    try testing.expectEqualStrings("2.75", s);
}

test "format: improper negative → mixed negative" {
    const num = Number.make(-17, 5);
    const s = try std.fmt.allocPrint(testing.allocator, "{f}", .{num});
    defer testing.allocator.free(s);

    try testing.expectEqualStrings("-3.4", s);
}

test "format: negative proper fraction" {
    const num = Number.make(-3, 8);
    const s = try std.fmt.allocPrint(testing.allocator, "{f}", .{num});
    defer testing.allocator.free(s);

    try testing.expectEqualStrings("-0.375", s);
}

test "format: whole number positive" {
    const num = Number.make(7, 1);
    const s = try std.fmt.allocPrint(testing.allocator, "{f}", .{num});
    defer testing.allocator.free(s);

    try testing.expectEqualStrings("7", s);
}

test "format: whole number negative" {
    const num = Number.make(-4, 1);
    const s = try std.fmt.allocPrint(testing.allocator, "{f}", .{num});
    defer testing.allocator.free(s);

    try testing.expectEqualStrings("-4", s);
}

test "format: decimal terminating 1/8" {
    const num = Number.make(1, 8);
    const s = try std.fmt.allocPrint(testing.allocator, "{f}", .{num});
    defer testing.allocator.free(s);

    try testing.expectEqualStrings("0.125", s);
}

test "format: decimal terminating 3/16" {
    const num = Number.make(3, 16);
    const s = try std.fmt.allocPrint(testing.allocator, "{f}", .{num});
    defer testing.allocator.free(s);

    try testing.expectEqualStrings("0.1875", s);
}

test "format: decimal terminating large" {
    const num = Number.make_simplify(123456, 64); // 1929.0
    const s = try std.fmt.allocPrint(testing.allocator, "{f}", .{num});
    defer testing.allocator.free(s);

    try testing.expectEqualStrings("1929", s);
}

test "format: decimal with leading zero in fraction part" {
    const num = Number.make(1, 16);
    const s = try std.fmt.allocPrint(testing.allocator, "{f}", .{num});
    defer testing.allocator.free(s);

    try testing.expectEqualStrings("0.0625", s); // проверка :0> заполнения нулями
}

test "format: very small fraction → обычная" {
    const num = Number.make(1, 7);
    const s = try std.fmt.allocPrint(testing.allocator, "{f}", .{num});
    defer testing.allocator.free(s);

    try testing.expectEqualStrings("1/7", s);
}

test "format: zero" {
    const num = Number.make_simplify(0, 5); // или 0/1 — не важно
    const s = try std.fmt.allocPrint(testing.allocator, "{f}", .{num});
    defer testing.allocator.free(s);

    try testing.expectEqualStrings("0", s);
}

test "parse: fraction test" {
    try testing.expectEqual(Number.make(2, 3), try Number.parse("2/3"));
}

test "parse: decimal test" {
    try testing.expectEqual(Number.make(1, 2), try Number.parse("0.5"));
}

test "parse: mixed test" {
    try testing.expectEqual(Number.make(4, 3), try Number.parse("1 1/3"));
}

test "parse: empty test" {
    try testing.expectError(error.IsEmpty, Number.parse(""));
}

test "parse: negative test" {
    try testing.expectEqual(Number.make(-2, 3), try Number.parse("-2/3"));
}

test "parse: simple fraction" {
    try testing.expectEqual(Number.make(3, 4), try Number.parse("3/4"));
}

test "parse: fraction with numerator 1" {
    try testing.expectEqual(Number.make(1, 5), try Number.parse("1/5"));
}

test "parse: fraction with numerator 0" {
    try testing.expectEqual(Number.make(0, 1), try Number.parse("0/7"));
}

// Целые числа (без дробной части)
test "parse: just integer positive" {
    try testing.expectEqual(Number.make(42, 1), try Number.parse("42"));
}

test "parse: just integer zero" {
    try testing.expectEqual(Number.make(0, 1), try Number.parse("0"));
}

test "parse: just integer negative" {
    try testing.expectEqual(Number.make(-17, 1), try Number.parse("-17"));
}

// Смешанные числа (целая часть + дробь)
test "parse: mixed number with space" {
    try testing.expectEqual(Number.make(5, 6), try Number.parse("0 5/6"));
}

test "parse: mixed number larger whole" {
    try testing.expectEqual(Number.make(13, 8), try Number.parse("1 5/8"));
}

test "parse: mixed negative" {
    try testing.expectEqual(Number.make(-7, 2), try Number.parse("-3 1/2"));
}

// Пробелы и вариации формата
test "parse: extra space before fraction" {
    try testing.expectEqual(Number.make(2, 9), try Number.parse("0  2/9")); // два пробела
}

// Ошибочные форматы (должны возвращать error.FormatError)
test "parse: invalid - trailing junk" {
    try testing.expectError(error.FormatError, Number.parse("3/4abc"));
}

test "parse: invalid - missing numerator" {
    try testing.expectError(error.FormatError, Number.parse("5 /")); // после пробела ничего
}

test "parse: invalid - missing denominator" {
    try testing.expectError(error.FormatError, Number.parse("2 3/"));
}

test "parse: invalid - double slash" {
    try testing.expectError(error.FormatError, Number.parse("1 4//5"));
}

test "parse: invalid - letters in number" {
    try testing.expectError(error.FormatError, Number.parse("1a 2/3"));
}

test "parse: invalid - only slash" {
    try testing.expectError(error.FormatError, Number.parse("/"));
}

test "parse: invalid - empty after sign" {
    try testing.expectError(error.FormatError, Number.parse("-"));
}

test "parse: invalid - multiple signs" {
    try testing.expectError(error.FormatError, Number.parse("--3/4"));
}

// Пограничные / интересные случаи
test "parse: very large numbers" {
    // Если Number использует u64 → должно парситься, если не переполняется
    try testing.expectEqual(Number.make(999999999999999999, 1), try Number.parse("999999999999999999"));
}

test "parse: fraction with leading zero in numerator" {
    try testing.expectEqual(Number.make(5, 12), try Number.parse("05/12"));
}

test "parse: fraction with leading zero in denominator" {
    try testing.expectEqual(Number.make(7, 8), try Number.parse("7/08"));
}

test "parse: simple decimal 0.point" {
    try testing.expectEqual(Number.make(1, 2), try Number.parse("0.5"));
}

test "parse: decimal without leading zero" {
    try testing.expectEqual(Number.make(3, 4), try Number.parse(".75"));
}

test "parse: decimal with one digit after point" {
    try testing.expectEqual(Number.make(2, 1), try Number.parse("2.0"));
}

test "parse: decimal negative" {
    try testing.expectEqual(Number.make(-7, 10), try Number.parse("-0.7"));
}

test "parse: decimal with multiple digits" {
    try testing.expectEqual(Number.make(123, 4), try Number.parse("30.75"));
    // 30.75 = 123/4
}

test "parse: decimal ends with point (should be integer)" {
    try testing.expectEqual(Number.make(5, 1), try Number.parse("5."));
    try testing.expectEqual(Number.make(-42, 1), try Number.parse("-42."));
}

test "parse: decimal with trailing zeros" {
    try testing.expectEqual(Number.make(1, 8), try Number.parse("0.12500"));
    try testing.expectEqual(Number.make(3, 2), try Number.parse("1.5000"));
}

// ───────────────────────────────────────────────
// Смешанные случаи: целая часть + десятичная дробь
// ───────────────────────────────────────────────

test "parse: integer + decimal" {
    try testing.expectEqual(Number.make(17, 10), try Number.parse("1.7"));
}

test "parse: large whole + decimal" {
    try testing.expectEqual(Number.make(583, 100), try Number.parse("5.83"));
}

test "parse: negative decimal with whole part" {
    try testing.expectEqual(Number.make(-131, 20), try Number.parse("-6.55"));
    // -6.55 = -131/20
}

// ───────────────────────────────────────────────
// Пограничные / нестандартные варианты десятичной записи
// ───────────────────────────────────────────────

test "parse: decimal with only point and zero" {
    try testing.expectEqual(Number.make(0, 1), try Number.parse("0.0"));
    try testing.expectEqual(Number.make(0, 1), try Number.parse(".0"));
}

test "parse: many digits after decimal" {
    // 0.0625 = 1/16
    try testing.expectEqual(Number.make(1, 16), try Number.parse("0.0625"));
}

test "parse: decimal starts with many zeros" {
    try testing.expectEqual(Number.make(1, 200), try Number.parse("0.005"));
}

// ───────────────────────────────────────────────
// Ошибочные / недопустимые десятичные форматы
// ───────────────────────────────────────────────

test "parse: invalid decimal - two points" {
    try testing.expectError(error.FormatError, Number.parse("1.2.3"));
    try testing.expectError(error.FormatError, Number.parse("0..5"));
}

test "parse: invalid decimal - point at end + junk" {
    try testing.expectError(error.FormatError, Number.parse("3. abc"));
}

test "parse: invalid - letters in decimal part" {
    try testing.expectError(error.FormatError, Number.parse("1.2a"));
    try testing.expectError(error.FormatError, Number.parse("0.4e2")); // научная нотация пока не поддерживается
}

test "parse: invalid - multiple points with sign" {
    try testing.expectError(error.FormatError, Number.parse("-1.2.5"));
}

test "parse: only point" {
    try testing.expectError(error.FormatError, Number.parse("."));
    try testing.expectError(error.FormatError, Number.parse("-."));
}

test "parse: looks like fraction but has point" {
    // Если ваша реализация сначала ищет пробел → '/'
    // то такие строки должны парситься как десятичные, а не как дробь
    try testing.expectEqual(Number.make(5, 2), try Number.parse("2.5"));
    try testing.expectError(error.FormatError, Number.parse("2 . 5")); // пробелы вокруг точки — обычно ошибка
}
