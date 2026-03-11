const std = @import("std");
const math = std.math;
const number = struct{
    numerator: i64,
    denominator: u64,
    fn make(numerator: i64, denominator: u64) !number {
        return number{.numerator = numerator, .denominator = denominator,};
    }
    fn isCorrect(self: *number) bool {
        return denominator != 0;
    }
    fn simplify_inplace(self: *number) void {
        const greatest_common_diviser = math.gcd(@as(u64, @abs(self.numerator)), self.denominator);
        self.denominator /= greatest_common_diviser;
        self.numerator /= greatest_common_diviser;
    }
    fn simplify(self: number) number {
        self.simplify();
        return self;
    }
    fn add_inplace(self: *number, other: *number) number {
        const 
    }
};
