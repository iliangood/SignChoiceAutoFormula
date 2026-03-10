const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const libfractions_mod = b.createModule(.{
        .root_source_file = null,
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    const libfractions = b.addLibrary(.{
        .name = "fractions",
        .linkage = .static,
        .root_module = libfractions_mod,
    });
    libfractions.root_module.addCSourceFiles(.{ .files = &.{
        "src/fractions.c",
    }, .flags = &.{
        "-Wall",
        "-Wextra",
        "-g",
    } });
    libfractions.root_module.addIncludePath(b.path("include"));

    b.installArtifact(libfractions);

    const main_mod = b.createModule(.{
        .root_source_file = b.path("main.zig"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    const exe = b.addExecutable(.{
        .name = "main",
        .root_module = main_mod,
    });


    exe.root_module.addIncludePath(b.path("include"));

    exe.root_module.linkLibrary(libfractions);

    b.installArtifact(exe);

    const cmd_run = b.addRunArtifact(exe);
    cmd_run.step.dependOn(b.getInstallStep());
    if (b.args) |args| cmd_run.addArgs(args);

    const run_step = b.step("run", "rum main");
    run_step.dependOn(&cmd_run.step);

    const tests_step = b.step("test", "run tests");

    const unit_tests = b.addTest(.{
        .root_module = main_mod,
    });
    const run_unit_tests = b.addRunArtifact(unit_tests);
    tests_step.dependOn(&run_unit_tests.step);
}
